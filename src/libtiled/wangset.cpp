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
    int shift = (index * 8);

    int color = (mId >> shift) & 0xf;

    return color;
}

int WangId::cornerColor(int index) const
{
    int shift = (index * 8) + 4;

    int color = (mId >> shift) & 0xf;

    return color;
}

int WangId::indexColor(int index) const
{
    if (index & 1)
        return cornerColor(index / 2);
    else
        return edgeColor(index / 2);
}

void WangId::setEdgeColor(int index, unsigned value)
{
    value = value & 0xf;
    mId &= ~(0xf << (index * 8));
    mId |= value << (index * 8);
}

void WangId::setCornerColor(int index, unsigned value)
{
    value = value & 0xf;
    mId &= ~(0xf << (index * 8 + 4));
    mId |= value << (index * 8 + 4);
}

void WangId::setIndexColor(int index, unsigned value)
{
    if (index & 1)
        setCornerColor(index / 2, value);
    else
        setEdgeColor(index / 2, value);
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

QVector<WangId> WangId::variations(int edgeColors, int cornerColors) const
{
    QVector<WangId> wangIds;

    auto i = variationsBegin(edgeColors, cornerColors);
    auto end = variationsEnd(edgeColors, cornerColors);

    if (i == end) {
        wangIds.append(mId);
        return wangIds;
    }
    while (true) {
        wangIds.append(*i);

        if (++i == end) {
            wangIds.append(*i);
            break;
        }
    }

    return wangIds;
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

    for (int i = 0; i < 4; ++i) {
        newWangId.setCornerColor(i, cornerColor(3-i));
    }

    mId = newWangId;
}

void WangId::flipVertically()
{
    flipHorizontally();
    rotate(2);
}

WangIdVariationIterator::WangIdVariationIterator(int edgeColors, int cornerColors, WangId wangId)
    : mCurrent(wangId)
    , mEdgeColors(edgeColors)
    , mCornerColors(cornerColors)
{
    if (mEdgeColors > 1) {
        for (int i = 0; i < 4; ++i)
            if (!wangId.edgeColor(i))
                mZeroSpots.append(i*2);
    }

    if (mCornerColors > 1) {
        for (int i = 0; i < 4; ++i)
            if (!wangId.cornerColor(i))
                mZeroSpots.append(i*2 + 1);
    }
}

WangIdVariationIterator &WangIdVariationIterator::operator ++()
{
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

WangIdVariationIterator WangId::variationsBegin(int edgeColors, int cornerColors) const
{
    return WangIdVariationIterator(edgeColors, cornerColors, mId);
}

WangIdVariationIterator WangId::variationsEnd(int edgeColors, int cornerColors) const
{
    WangId end = mId;

    if (edgeColors > 1) {
        for (int i = 0; i < 4; ++i)
            if (!edgeColor(i))
                end.setEdgeColor(i, edgeColors);
    }

    if (cornerColors > 1) {
        for (int i = 0; i < 4; ++i)
            if (!cornerColor(i))
                end.setCornerColor(i, cornerColors);
    }

    return WangIdVariationIterator(edgeColors, cornerColors, end);
}

void WangTile::translate(int map[])
{
    int mask = (mFlippedHorizontally << 2)
            | (mFlippedVertically << 1)
            | (mFlippedAntiDiagonally);

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

WangSet::WangSet(Tileset *tileset,
                 int edgeColors,
                 int cornerColors,
                 QString name,
                 int imageTileId):
    Object(Object::WangSetType),
    mTileset(tileset),
    mName(std::move(name)),
    mImageTileId(imageTileId),
    mEdgeColors(edgeColors),
    mCornerColors(cornerColors)
{
}

QList<Tile *> WangSet::tilesChangedOnSetEdgeColors(int newEdgeColors)
{
    QList<Tile *> tiles;

    int previousEdgeColors = mEdgeColors;
    mEdgeColors = newEdgeColors;

    for (auto i = mTileInfoToWangId.cbegin(); i != mTileInfoToWangId.cend(); ++i) {
        if (!wangIdIsValid(i.value())) {
            int tileId = i.key() & 0x1fffffff;
            tiles.append(mTileset->tileAt(tileId));
        }
    }

    mEdgeColors = previousEdgeColors;

    return tiles;
}

QList<Tile *> WangSet::tilesChangedOnSetCornerColors(int newCornerColors)
{
    QList<Tile *> tiles;

    int previousCornerColors = mCornerColors;
    mCornerColors = newCornerColors;

    for (auto i = mTileInfoToWangId.cbegin(); i != mTileInfoToWangId.cend(); ++i) {
        if (!wangIdIsValid(i.value())) {
            int tileId = i.key() & 0x1fffffff;
            tiles.append(mTileset->tileAt(tileId));
        }
    }

    mCornerColors = previousCornerColors;

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

    mWangIdToWangTile.insert(wangTile.wangId(), wangTile);
    mTileInfoToWangId.insert(wangTileToTileInfo(wangTile), wangTile.wangId());
}

void WangSet::removeTile(Tile *tile)
{
    removeWangTile(WangTile(tile, 0));
}

void WangSet::removeCell(const Cell &cell)
{
    removeWangTile(WangTile(cell, 0));
}

void WangSet::removeWangTile(const WangTile &wangTile)
{
    WangId wangId = mTileInfoToWangId.take(wangTileToTileInfo(wangTile));

    WangTile w = wangTile;
    w.setWangId(wangId);

    mWangIdToWangTile.remove(wangId, w);
}

WangTile WangSet::findMatchingWangTile(WangId wangId) const
{
    auto potentials = findMatchingWangTiles(wangId);

    if (potentials.length() > 0)
        return potentials[qrand() % potentials.length()];
    else
        return WangTile();
}

QList<WangTile> WangSet::findMatchingWangTiles(WangId wangId) const
{
    if (wangId == 0)
        return mWangIdToWangTile.values();

    QList<WangTile> list;
    QVector<WangId> wangIds = wangId.variations(mEdgeColors, mCornerColors);

    for (WangId id : wangIds)
        list.append(mWangIdToWangTile.values(id));

    return list;
}

WangId WangSet::wangIdFromSurrounding(WangId surroundingWangIds[]) const
{
    unsigned id = 0;

    if (mEdgeColors > 1) {
        for (int i = 0; i < 4; ++i)
            id |= (surroundingWangIds[i*2].edgeColor((2 + i) % 4)) << (i*8);
    }

    if (mCornerColors > 1) {
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
    return mTileInfoToWangId.value(tile->id());
}

WangId WangSet::wangIdOfCell(const Cell &cell) const
{
    return mTileInfoToWangId.value(cellToTileInfo(cell));
}

bool WangSet::wangIdIsValid(WangId wangId) const
{
    for (int i = 0; i < 4; ++i) {
        if (wangId.edgeColor(i) > mEdgeColors
                || wangId.cornerColor(i) > mCornerColors)
            return false;

        if (mEdgeColors <= 1)
            if (wangId.edgeColor(i))
                return false;

        if (mCornerColors <= 1)
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

    auto i = wangId.variationsBegin(mEdgeColors, mCornerColors);
    auto end = wangId.variationsEnd(mEdgeColors, mCornerColors);

    while (true) {
        if (wangIdIsUsed(*i))
            return true;

        if (++i == end) {
            if (wangIdIsUsed(*i))
                return true;
            break;
        }
    }

    return false;
}

bool WangSet::isComplete() const
{
    if (isEmpty())
        return false;
    return (unsigned) mWangIdToWangTile.uniqueKeys().size() == completeSetSize();
}

unsigned WangSet::completeSetSize() const
{
    return qPow(mEdgeColors, 4) * qPow(mCornerColors, 4);
}

WangId WangSet::templateWangIdAt(unsigned n) const
{
    unsigned wangId = 0;
    //number of permutations of a corner and edge together.
    int cornerEdgePermutations = mEdgeColors * mCornerColors;

    for (int i = 7; i >= 0; --i) {
        //this is the number of permutations possible bellow this point in the wangId
        int belowPermutations = qPow(cornerEdgePermutations, i/2) * ((i&1)? mEdgeColors : 1);
        int value = n / belowPermutations;
        n -= value * belowPermutations;

        wangId |= value << i * 4;
    }

    //before this is like a base 10 range (0 - 9) where we want (1 - 10) for each digit
    wangId += 0x11111111;
    //If edges/corners don't have variations then those spots should be wild.
    if (mEdgeColors <= 1)
        wangId &= 0xf0f0f0f0;
    if (mCornerColors <= 1)
        wangId &= 0x0f0f0f0f;

    return wangId;
}

WangSet *WangSet::clone(Tileset *tileset) const
{
    WangSet *c = new WangSet(tileset, mEdgeColors, mCornerColors, mName, mImageTileId);

    c->mWangIdToWangTile = mWangIdToWangTile;
    c->mTileInfoToWangId = mTileInfoToWangId;

    return c;
}
