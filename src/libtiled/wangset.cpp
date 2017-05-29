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

#include "wangset.h"

#include <QStack>
#include <QPoint>

using namespace Tiled;

WangSet::WangSet(Tileset *tileset,
                 int edgeColors,
                 int cornerColors,
                 QString name,
                 int imageTileId):
    Object(Object::TerrainType), //for now, will add unique type soon
    mTileSet(tileset),
    mEdgeColors(edgeColors),
    mCornerColors(cornerColors),
    mName(std::move(name)),
    mImageTileId(imageTileId),
    mWangIdToTile(QMultiMap<WangId, Tile*>()),
    mTileIdToWangId(QMap<int, WangId>())
{
}

void WangSet::addTile(Tile *tile, WangId wangId)
{
    Q_ASSERT(tile->tileset() == mTileSet);

    for (int i = 0; i < 4; ++i) {
        Q_ASSERT(wangId.getColor(i,true) <= mEdgeColors);
    }

    for (int i = 0; i < 4; ++i) {
        Q_ASSERT(wangId.getColor(i,false) <= mCornerColors);
    }

    mWangIdToTile.insert(wangId, tile);
    mTileIdToWangId.insert(tile->id(), wangId);
}

Tile *WangSet::getMatchingTile(WangId wangId) const
{
    auto potentials = getAllTiles(wangId);

    if (potentials.length() > 0)
        return potentials[qrand() % potentials.length()];
    else
        return NULL;
}

QList<Tile*> WangSet::getAllTiles(WangId wangId) const
{
    QList<Tile*> list;

    //Stores the space of a wild card, followed by how many colors that space can have.
    QVector<QPoint> wildCards;

    if (mEdgeColors > 0) {
        for (int i = 0; i < 4; ++i) {
            if (!wangId.getColor(i,true)) {
                wildCards.append(QPoint(i * 8, mEdgeColors));
            }
        }
    }

    if (mCornerColors > 0) {
        for (int i = 0; i < 4; ++i) {
            if (!wangId.getColor(i,false)) {
                wildCards.append(QPoint(i * 8 + 4, mCornerColors));
            }
        }
    }

    if (wildCards.isEmpty()) {
        list.append(mWangIdToTile.values(wangId));
    } else {
        QStack<QPoint> stack;

        for (int i = 0; i < wildCards.size(); ++i) {
            stack.push(wildCards[i]);
        }

        int max = wildCards.size();

        while (!stack.isEmpty()) {
            if (stack.size() == max) {
                int idVariation = 0;

                for (int i = 0; i < max; ++i) {
                    idVariation |= stack[i].y() << stack[i].x();
                }

                list.append(mWangIdToTile.values(idVariation | wangId.id()));

                QPoint top = stack.pop();
                top.setY(top.y() - 1);
                if (top.y() > 0)
                    stack.push(top);
            } else {
                QPoint top = stack.pop();
                top.setY(top.y() - 1);
                if (top.y() > 0) {
                    stack.push(top);

                    for (int i = stack.size(); i < max; ++i) {
                        stack.push(wildCards[i]);
                    }
                }
            }
        }
    }

    return list;
}

WangId WangSet::getWangIdOfTile(Tile *tile) const
{
    if (tile->tileset() == mTileSet && mTileIdToWangId.contains(tile->id()))
        return mTileIdToWangId.value(tile->id());
    else
        return WangId(0);
}
