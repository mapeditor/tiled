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

    newWangId.setEdgeColor(2, edgeColor(6));
    newWangId.setEdgeColor(6, edgeColor(2));

    for (int i = 1; i < 8; i+=2)
        newWangId.setCornerColor(i, edgeColor(8 - i));

    mId = newWangId;
}

void WangId::flipVertically()
{
    flipHorizontally();
    rotate(2);
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

void WangSet::addTile(Tile *tile, WangId wangId)
{
    Q_ASSERT(tile->tileset() == mTileset);
    Q_ASSERT(wangIdIsValid(wangId));

    mWangIdToWangTile.insert(wangId, WangTile(tile, wangId));
    mTileInfoToWangId.insert(tile->id(), wangId);
}

void WangSet::addCell(const Cell &cell, WangId wangId)
{
    Q_ASSERT(cell.tileset() == mTileset);
    Q_ASSERT(wangIdIsValid(wangId));

    mWangIdToWangTile.insert(wangId, WangTile(cell, wangId));
    mTileInfoToWangId.insert(cellToTileInfo(cell), wangId);
}

void WangSet::addWangTile(const WangTile &wangTile)
{
    Q_ASSERT(wangTile.tile()->tileset() == mTileset);
    Q_ASSERT(wangIdIsValid(wangTile.wangId()));

    mWangIdToWangTile.insert(wangTile.wangId(), wangTile);
    mTileInfoToWangId.insert(wangTileToTileInfo(wangTile), wangTile.wangId());
}

WangTile WangSet::findMatchingWangTile(WangId wangId) const
{
    auto potentials = findMatchingWangTiles(wangId);

    if (potentials.length() > 0)
        return potentials[qrand() % potentials.length()];
    else
        return WangTile();
}

struct WangWildCard
{
    int index, colorCount;
};

QList<WangTile> WangSet::findMatchingWangTiles(WangId wangId) const
{
    QList<WangTile> list;

    //Stores the space of a wild card, followed by how many colors that space can have.
    QVector<WangWildCard> wildCards;

    if (mEdgeColors > 0) {
        for (int i = 0; i < 4; ++i) {
            if (!wangId.edgeColor(i)) {
                WangWildCard w;
                w.index = i * 8;
                w.colorCount = mEdgeColors;

                wildCards.append(w);
            }
        }
    }

    if (mCornerColors > 0) {
        for (int i = 0; i < 4; ++i) {
            if (!wangId.cornerColor(i)) {
                WangWildCard w;
                w.index = i * 8 + 4;
                w.colorCount = mCornerColors;

                wildCards.append(w);
            }
        }
    }

    if (wildCards.isEmpty()) {
        list.append(mWangIdToWangTile.values(wangId));
    } else {
        QStack<WangWildCard> stack;

        stack += wildCards;

        int max = wildCards.size();

        while (!stack.isEmpty()) {
            if (stack.size() == max) {
                int idVariation = 0;

                for (int i = 0; i < max; ++i)
                    idVariation |= stack[i].colorCount << stack[i].index;

                list.append(mWangIdToWangTile.values(idVariation | wangId));

                WangWildCard top = stack.pop();
                top.colorCount -= 1;
                if (top.colorCount > 0)
                    stack.push(top);
            } else {
                WangWildCard top = stack.pop();
                top.colorCount -= 1;
                if (top.colorCount > 0) {
                    stack.push(top);

                    for (int i = stack.size(); i < max; ++i)
                        stack.push(wildCards[i]);
                }
            }
        }
    }

    return list;
}

Cell WangSet::findMatchingCell(WangId wangId) const
{
    WangTile wangTile = findMatchingWangTile(wangId);

    return wangTile.makeCell();
}

WangId WangSet::wangIdFromSurrounding(WangId surroundingWangIds[]) const
{
    unsigned id = 0;

    if (mEdgeColors > 0) {
        for (int i = 0; i < 4; ++i)
            id |= (surroundingWangIds[i*2].edgeColor((2 + i) % 4)) << (i*8);
    }

    if (mCornerColors > 0) {
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

    WangId wangId = wangIdFromSurrounding(wangIds);

    return wangId;
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
    }

    return true;
}

WangSet *WangSet::clone(Tileset *tileset) const
{
    WangSet *c = new WangSet(tileset, mEdgeColors, mCornerColors, mName, mImageTileId);

    c->mWangIdToWangTile = mWangIdToWangTile;
    c->mTileInfoToWangId = mTileInfoToWangId;

    return c;
}
