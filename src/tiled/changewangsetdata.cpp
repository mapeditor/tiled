/*
 * changewangsetdata.cpp
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

#include "changewangsetdata.h"

#include "tileset.h"
#include "tilesetdocument.h"
#include "tilesetwangsetmodel.h"
#include "changetilewangid.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Internal;

ChangeWangSetEdges::ChangeWangSetEdges(TilesetDocument *tilesetDocument,
                                       int index,
                                       int newValue)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Wang Set edge count"))

    , mTilesetDocument(tilesetDocument)
    , mWangSetModel(tilesetDocument->wangSetModel())
    , mIndex(index)
    , mOldValue(tilesetDocument->tileset()->wangSet(index)->edgeColors())
    , mNewValue(newValue)
{
    //when edge size changes, all tiles with wangIds need to be updated.
    WangSet *wangSet = mTilesetDocument->tileset()->wangSet(index);
    Q_ASSERT(wangSet);
    mAffectedTiles = wangSet->tilesWithWangId();

    if (mNewValue < mOldValue) {
        //when the size is reduced, some wang assignments can be lost.
        const QList<Tile *> &changedTiles = wangSet->tilesChangedOnSetEdgeColors(mNewValue);

        if (!changedTiles.isEmpty()) {
            QVector<ChangeTileWangId::WangIdChange> changes;

            for (Tile *tile : changedTiles)
                changes.append(ChangeTileWangId::WangIdChange(wangSet->wangIdOfTile(tile), 0, tile));

            new ChangeTileWangId(mTilesetDocument, wangSet, changes, this);
        }

        for (int i = mOldValue; i > mNewValue; --i) {
            WangColorChange w;
            w.index = i;
            w.wangColor = wangSet->wangColorOfEdge(i);

            mRemovedWangColors.append(w);
        }
    }
}

void ChangeWangSetEdges::undo()
{
    mWangSetModel->setWangSetEdges(mIndex, mOldValue);

    if (!mAffectedTiles.isEmpty())
        emit mTilesetDocument->tileWangSetChanged(mAffectedTiles);

    WangSet *wangSet = mTilesetDocument->tileset()->wangSet(mIndex);

    for (WangColorChange w : mRemovedWangColors) {
        WangColor *wangColor = wangSet->wangColorOfEdge(w.index);
        wangColor->setName(w.wangColor->name());
        wangColor->setImageId(w.wangColor->imageId());
        wangColor->setColor(w.wangColor->color());
        wangColor->setProbability(w.wangColor->probability());
    }

    QUndoCommand::undo();
}

void ChangeWangSetEdges::redo()
{
    mWangSetModel->setWangSetEdges(mIndex, mNewValue);

    if (!mAffectedTiles.isEmpty())
        emit mTilesetDocument->tileWangSetChanged(mAffectedTiles);

    QUndoCommand::redo();
}

ChangeWangSetCorners::ChangeWangSetCorners(TilesetDocument *tilesetDocument,
                                           int index,
                                           int newValue)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Wang Set corner count"))
    , mTilesetDocument(tilesetDocument)
    , mWangSetModel(tilesetDocument->wangSetModel())
    , mIndex(index)
    , mOldValue(tilesetDocument->tileset()->wangSet(index)->cornerColors())
    , mNewValue(newValue)
{
    //when corner size changes, all tiles with wangIds need to be updated.
    WangSet *wangSet = mTilesetDocument->tileset()->wangSet(index);
    Q_ASSERT(wangSet);
    mAffectedTiles = wangSet->tilesWithWangId();

    if (mNewValue < mOldValue) {
        //when the size is reduced, some wang assignments can be lost.
        const QList<Tile *> &changedTiles = wangSet->tilesChangedOnSetCornerColors(mNewValue);

        if (!changedTiles.isEmpty()) {
            QVector<ChangeTileWangId::WangIdChange> changes;

            for (Tile *tile : changedTiles)
                changes.append(ChangeTileWangId::WangIdChange(wangSet->wangIdOfTile(tile), 0, tile));

            new ChangeTileWangId(mTilesetDocument, wangSet, changes, this);
        }

        for (int i = mOldValue; i > mNewValue; --i) {
            WangColorChange w;
            w.index = i;
            w.wangColor = wangSet->wangColorOfCorner(i);

            mRemovedWangColors.append(w);
        }
    }
}

void ChangeWangSetCorners::undo()
{
    mWangSetModel->setWangSetCorners(mIndex, mOldValue);

    if (!mAffectedTiles.isEmpty())
        emit mTilesetDocument->tileWangSetChanged(mAffectedTiles);

    WangSet *wangSet = mTilesetDocument->tileset()->wangSet(mIndex);

    for (WangColorChange w : mRemovedWangColors) {
        WangColor *wangColor = wangSet->wangColorOfCorner(w.index);
        wangColor->setName(w.wangColor->name());
        wangColor->setImageId(w.wangColor->imageId());
        wangColor->setColor(w.wangColor->color());
        wangColor->setProbability(w.wangColor->probability());
    }

    QUndoCommand::undo();
}

void ChangeWangSetCorners::redo()
{
    mWangSetModel->setWangSetCorners(mIndex, mNewValue);

    if (!mAffectedTiles.isEmpty())
        emit mTilesetDocument->tileWangSetChanged(mAffectedTiles);

    QUndoCommand::redo();
}

SetWangSetImage::SetWangSetImage(TilesetDocument *tilesetDocument, int index, int tileId)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Set Wang Set Image"))
    , mWangSetModel(tilesetDocument->wangSetModel())
    , mIndex(index)
    , mOldImageTileId(tilesetDocument->tileset()->wangSet(index)->imageTileId())
    , mNewImageTileId(tileId)
{
}

void SetWangSetImage::undo()
{
    mWangSetModel->setWangSetImage(mIndex, mOldImageTileId);
}

void SetWangSetImage::redo()
{
    mWangSetModel->setWangSetImage(mIndex, mNewImageTileId);
}
