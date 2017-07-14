/*
 * changewangsetdata.cpp
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

#include "changewangsetdata.h"

#include "wangset.h"
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
    }
}

void ChangeWangSetEdges::undo()
{
    mWangSetModel->setWangSetEdges(mIndex, mOldValue);

    if (!mAffectedTiles.isEmpty())
        emit mTilesetDocument->tileWangSetChanged(mAffectedTiles);

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
    }

}

void ChangeWangSetCorners::undo()
{
    mWangSetModel->setWangSetCorners(mIndex, mOldValue);

    if (!mAffectedTiles.isEmpty())
        emit mTilesetDocument->tileWangSetChanged(mAffectedTiles);

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
