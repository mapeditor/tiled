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

#include "qtcompat_p.h"

using namespace Tiled;

ChangeWangSetEdgeCount::ChangeWangSetEdgeCount(TilesetDocument *tilesetDocument,
                                               WangSet *wangSet,
                                               int newValue)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Wang Set edge count"))

    , mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
    , mOldValue(wangSet->edgeColorCount())
    , mNewValue(newValue)
{
    // when edge size changes, all tiles with wangIds need to be updated.
    if (mNewValue < mOldValue) {
        // when the size is reduced, some wang assignments can be lost.
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

void ChangeWangSetEdgeCount::undo()
{
    mTilesetDocument->wangSetModel()->setWangSetEdges(mWangSet, mOldValue);

    for (const WangColorChange &w : qAsConst(mRemovedWangColors)) {
        WangColor *wangColor = mWangSet->edgeColorAt(w.index).data();
        wangColor->setName(w.wangColor->name());
        wangColor->setImageId(w.wangColor->imageId());
        wangColor->setColor(w.wangColor->color());
        wangColor->setProbability(w.wangColor->probability());
    }

    QUndoCommand::undo();
}

void ChangeWangSetEdgeCount::redo()
{
    mTilesetDocument->wangSetModel()->setWangSetEdges(mWangSet, mNewValue);

    QUndoCommand::redo();
}


ChangeWangSetCornerCount::ChangeWangSetCornerCount(TilesetDocument *tilesetDocument,
                                                   WangSet *wangSet,
                                                   int newValue)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Wang Set corner count"))
    , mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
    , mOldValue(wangSet->cornerColorCount())
    , mNewValue(newValue)
{
    // when corner size changes, all tiles with wangIds need to be updated.
    if (mNewValue < mOldValue) {
        // when the size is reduced, some wang assignments can be lost.
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

void ChangeWangSetCornerCount::undo()
{
    mTilesetDocument->wangSetModel()->setWangSetCorners(mWangSet, mOldValue);

    for (const WangColorChange &w : qAsConst(mRemovedWangColors)) {
        WangColor *wangColor = mWangSet->cornerColorAt(w.index).data();
        wangColor->setName(w.wangColor->name());
        wangColor->setImageId(w.wangColor->imageId());
        wangColor->setColor(w.wangColor->color());
        wangColor->setProbability(w.wangColor->probability());
    }

    QUndoCommand::undo();
}

void ChangeWangSetCornerCount::redo()
{
    mTilesetDocument->wangSetModel()->setWangSetCorners(mWangSet, mNewValue);

    QUndoCommand::redo();
}


RemoveWangSetColor::RemoveWangSetColor(TilesetDocument *tilesetDocumnet, WangSet *wangSet, int color, bool isEdge)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Remove Wang Color"))
    , mTilesetDocument(tilesetDocumnet)
    , mWangSet(wangSet)
    , mColor(color)
    , mIsEdge(isEdge)
{
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
    TilesetWangSetModel *wangSetModel = mTilesetDocument->wangSetModel();

    if (mExtraWangColor) {
        if (mRemovedWangColor->colorIndex() > mExtraWangColor->colorIndex()) {
            wangSetModel->insertWangColor(mWangSet, mExtraWangColor);
            wangSetModel->insertWangColor(mWangSet, mRemovedWangColor);
        } else {
            wangSetModel->insertWangColor(mWangSet, mRemovedWangColor);
            wangSetModel->insertWangColor(mWangSet, mExtraWangColor);
        }
    } else {
        wangSetModel->insertWangColor(mWangSet, mRemovedWangColor);
    }

    QUndoCommand::undo();
}

void RemoveWangSetColor::redo()
{
    mTilesetDocument->wangSetModel()->removeWangColorAt(mWangSet, mColor, mIsEdge);

    QUndoCommand::redo();
}


SetWangSetImage::SetWangSetImage(TilesetDocument *tilesetDocument,
                                 WangSet *wangSet,
                                 int tileId,
                                 QUndoCommand *parent)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Set Wang Set Image"),
                   parent)
    , mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
    , mOldImageTileId(wangSet->imageTileId())
    , mNewImageTileId(tileId)
{
}

void SetWangSetImage::undo()
{
    mTilesetDocument->wangSetModel()->setWangSetImage(mWangSet, mOldImageTileId);
}

void SetWangSetImage::redo()
{
    mTilesetDocument->wangSetModel()->setWangSetImage(mWangSet, mNewImageTileId);
}
