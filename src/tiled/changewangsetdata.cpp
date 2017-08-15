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
    , mOldValue(tilesetDocument->tileset()->wangSet(index)->edgeColorCount())
    , mNewValue(newValue)
{
    //when edge size changes, all tiles with wangIds need to be updated.
    WangSet *wangSet = mTilesetDocument->tileset()->wangSet(index);
    Q_ASSERT(wangSet);

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
            w.wangColor = wangSet->edgeColorAt(i);

            mRemovedWangColors.append(w);
        }

        if (mNewValue == 1) {
            WangColorChange w;
            w.index = 1;
            w.wangColor = wangSet->edgeColorAt(1);

            mRemovedWangColors.append(w);
        }
    }
}

void ChangeWangSetEdges::undo()
{
    mWangSetModel->setWangSetEdges(mIndex, mOldValue);

    WangSet *wangSet = mTilesetDocument->tileset()->wangSet(mIndex);

    for (WangColorChange w : mRemovedWangColors) {
        WangColor *wangColor = wangSet->edgeColorAt(w.index).data();
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
    , mOldValue(tilesetDocument->tileset()->wangSet(index)->cornerColorCount())
    , mNewValue(newValue)
{
    //when corner size changes, all tiles with wangIds need to be updated.
    WangSet *wangSet = mTilesetDocument->tileset()->wangSet(index);
    Q_ASSERT(wangSet);

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
            w.wangColor = wangSet->cornerColorAt(i);

            mRemovedWangColors.append(w);
        }

        if (mNewValue == 1) {
            WangColorChange w;
            w.index = 1;
            w.wangColor = wangSet->cornerColorAt(1);

            mRemovedWangColors.append(w);
        }
    }
}

void ChangeWangSetCorners::undo()
{
    mWangSetModel->setWangSetCorners(mIndex, mOldValue);

    WangSet *wangSet = mTilesetDocument->tileset()->wangSet(mIndex);

    for (WangColorChange w : mRemovedWangColors) {
        WangColor *wangColor = wangSet->cornerColorAt(w.index).data();
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

    QUndoCommand::redo();
}

RemoveWangSetColor::RemoveWangSetColor(TilesetDocument *tilesetDocumnet, int index, int color, bool isEdge)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Remove Wang Color"))
    , mTilesetDocument(tilesetDocumnet)
    , mWangSetModel(tilesetDocumnet->wangSetModel())
    , mIndex(index)
    , mColor(color)
    , mIsEdge(isEdge)
{
    WangSet *wangSet = mTilesetDocument->tileset()->wangSet(mIndex);

    Q_ASSERT(wangSet);

    if (mIsEdge) {
        mRemovedWangColor = wangSet->edgeColorAt(mColor);

        if (wangSet->edgeColorCount() == 2)
            mExtraWangColor = wangSet->edgeColorAt((mColor << 1) % 3);
        else
            mExtraWangColor = QSharedPointer<WangColor>();
    } else {
        mRemovedWangColor = wangSet->cornerColorAt(mColor);

        if (wangSet->cornerColorCount() == 2)
            mExtraWangColor = wangSet->cornerColorAt((mColor << 1) % 3);
        else
            mExtraWangColor = QSharedPointer<WangColor>();
    }

    QList<Tile *> changedTiles = wangSet->tilesChangedOnRemoveColor(mColor, mIsEdge);

    if (!changedTiles.isEmpty()) {
        QVector<ChangeTileWangId::WangIdChange> changes;

        for (Tile *tile : changedTiles) {
            WangId oldWangId = wangSet->wangIdOfTile(tile);
            WangId changedWangId = oldWangId;

            if (mIsEdge) {
                for (int i = 0; i < 4; ++i) {
                    int edgeColor = changedWangId.edgeColor(i);
                    if (edgeColor && (edgeColor == mColor || wangSet->edgeColorCount() == 2))
                        changedWangId.setEdgeColor(i, 0);
                    else if (edgeColor > mColor)
                        changedWangId.setEdgeColor(i, edgeColor - 1);
                }
            } else {
                for (int i = 0; i < 4; ++i) {
                    int cornerColor = changedWangId.cornerColor(i);
                    if (cornerColor && (cornerColor == mColor || wangSet->cornerColorCount() == 2))
                        changedWangId.setCornerColor(i, 0);
                    else if (cornerColor > mColor)
                        changedWangId.setCornerColor(i, cornerColor - 1);
                }
            }

            changes.append(ChangeTileWangId::WangIdChange(oldWangId,
                                                          changedWangId,
                                                          tile));
        }

        new ChangeTileWangId(mTilesetDocument, wangSet, changes, this);
    }
}

void RemoveWangSetColor::undo()
{
    if (mExtraWangColor) {
        if (mRemovedWangColor->colorIndex() > mExtraWangColor->colorIndex()) {
            mWangSetModel->insertWangColor(mIndex, mExtraWangColor);
            mWangSetModel->insertWangColor(mIndex, mRemovedWangColor);
        } else {
            mWangSetModel->insertWangColor(mIndex, mRemovedWangColor);
            mWangSetModel->insertWangColor(mIndex, mExtraWangColor);
        }
    } else {
        mWangSetModel->insertWangColor(mIndex, mRemovedWangColor);
    }

    QUndoCommand::undo();
}

void RemoveWangSetColor::redo()
{
    mWangSetModel->removeWangColorAt(mIndex, mColor, mIsEdge);

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
