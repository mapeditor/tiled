/*
 * changetilewangid.cpp
 * Copyright 2017, Benjamin Trotter <bdtrotte@ucsc.edu>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "changetilewangid.h"

#include "tilesetdocument.h"
#include "tile.h"

#include <QCoreApplication>

using namespace Tiled;

ChangeTileWangId::ChangeTileWangId()
    : mTilesetDocument(nullptr)
    , mWangSet(nullptr)
    , mMergeable(false)
{
    setText(QCoreApplication::translate("Undo Commands", "Change Tile Terrain"));
}

ChangeTileWangId::ChangeTileWangId(TilesetDocument *tilesetDocument,
                                   WangSet *wangSet,
                                   Tile *tile,
                                   WangId wangId)
    : mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
    , mMergeable(true)
{
    Q_ASSERT(mWangSet);
    setText(QCoreApplication::translate("Undo Commands", "Change Tile Terrain"));
    mChanges.append(WangIdChange(mWangSet->wangIdOfTile(tile), wangId, tile->id()));
}

ChangeTileWangId::ChangeTileWangId(TilesetDocument *tilesetDocument,
                                   WangSet *wangSet,
                                   const QVector<WangIdChange> &changes,
                                   QUndoCommand *parent)
    : QUndoCommand(parent)
    , mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
    , mChanges(changes)
    , mMergeable(true)
{
    setText(QCoreApplication::translate("Undo Commands", "Change Tile Terrain"));
}

void ChangeTileWangId::undo()
{
    if (mChanges.isEmpty())
        return;

    QList<Tile *> changedTiles;

    QVectorIterator<WangIdChange> changes(mChanges);
    changes.toBack();

    while (changes.hasPrevious()) {
        const WangIdChange &wangIdChange = changes.previous();

        if (Tile *tile = findTile(wangIdChange.tileId))
            changedTiles.append(tile);
        mWangSet->setWangId(wangIdChange.tileId, wangIdChange.from);
    }

    emit mTilesetDocument->tileWangSetChanged(changedTiles);
}

void ChangeTileWangId::redo()
{
    if (mChanges.isEmpty())
        return;

    QList<Tile *> changedTiles;

    for (const WangIdChange &wangIdChange : std::as_const(mChanges)) {
        if (Tile *tile = findTile(wangIdChange.tileId))
            changedTiles.append(tile);
        mWangSet->setWangId(wangIdChange.tileId, wangIdChange.to);
    }

    emit mTilesetDocument->tileWangSetChanged(changedTiles);
}

bool ChangeTileWangId::mergeWith(const QUndoCommand *other)
{
    if (!mMergeable)
        return false;

    auto o = static_cast<const ChangeTileWangId*>(other);
    if (o->mTilesetDocument && !(mTilesetDocument == o->mTilesetDocument &&
                                 mWangSet == o->mWangSet))
        return false;

    // suboptimal, could use a map to remove any unnecessary changes if the
    // same tile has multiple changes.
    mChanges += o->mChanges;

    mMergeable = o->mMergeable;

    return true;
}

QVector<ChangeTileWangId::WangIdChange> ChangeTileWangId::changesOnSetColorCount(
        const WangSet *wangSet, int colorCount)
{
    QVector<WangIdChange> changes;

    QHashIterator<int, WangId> it(wangSet->wangIdByTileId());
    while (it.hasNext()) {
        it.next();
        WangId newWangId = it.value();

        for (int i = 0; i < WangId::NumIndexes; ++i)
            if (newWangId.indexColor(i) > colorCount)
                newWangId.setIndexColor(i, 0);

        if (it.value() != newWangId)
            changes.append(WangIdChange(it.value(), newWangId, it.key()));
    }

    return changes;
}

QVector<ChangeTileWangId::WangIdChange> ChangeTileWangId::changesOnRemoveColor(
        const WangSet *wangSet, int removedColor)
{
    QVector<WangIdChange> changes;

    QHashIterator<int, WangId> it(wangSet->wangIdByTileId());
    while (it.hasNext()) {
        it.next();
        WangId newWangId = it.value();

        for (int i = 0; i < WangId::NumIndexes; ++i) {
            const int color = newWangId.indexColor(i);
            if (color == removedColor)
                newWangId.setIndexColor(i, 0);
            else if (color > removedColor)
                newWangId.setIndexColor(i, color - 1);
        }

        if (it.value() != newWangId)
            changes.append(WangIdChange(it.value(), newWangId, it.key()));
    }

    return changes;
}

void ChangeTileWangId::applyChanges(WangSet *wangSet, const QVector<WangIdChange> &changes)
{
    for (const WangIdChange &change : changes)
        wangSet->setWangId(change.tileId, change.to);
}

Tile *ChangeTileWangId::findTile(int tileId) const
{
    return mTilesetDocument->tileset()->findTile(tileId);
}
