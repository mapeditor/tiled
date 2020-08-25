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

ChangeWangSetColorCount::ChangeWangSetColorCount(TilesetDocument *tilesetDocument,
                                                 WangSet *wangSet,
                                                 int newValue)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Wang Set Color Count"))
    , mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
    , mOldValue(wangSet->colorCount())
    , mNewValue(newValue)
{
    // when edge size changes, all tiles with wangIds need to be updated.
    if (mNewValue < mOldValue) {
        // when the size is reduced, some wang assignments can be lost.
        const QList<Tile *> changedTiles = wangSet->tilesChangedOnSetColorCount(mNewValue);

        if (!changedTiles.isEmpty()) {
            QVector<ChangeTileWangId::WangIdChange> changes;

            for (Tile *tile : changedTiles)
                changes.append(ChangeTileWangId::WangIdChange(wangSet->wangIdOfTile(tile), 0, tile));

            new ChangeTileWangId(mTilesetDocument, wangSet, changes, this);
        }

        for (int i = mOldValue; i > mNewValue; --i) {
            WangColorChange w;
            w.index = i;
            w.wangColor = wangSet->colorAt(i);

            mRemovedWangColors.append(w);
        }
    }
}

void ChangeWangSetColorCount::undo()
{
    mTilesetDocument->wangSetModel()->setWangSetColorCount(mWangSet, mOldValue);

    for (const WangColorChange &w : qAsConst(mRemovedWangColors)) {
        WangColor &wangColor = *mWangSet->colorAt(w.index);
        wangColor.setName(w.wangColor->name());
        wangColor.setImageId(w.wangColor->imageId());
        wangColor.setColor(w.wangColor->color());
        wangColor.setProbability(w.wangColor->probability());
    }

    QUndoCommand::undo();
}

void ChangeWangSetColorCount::redo()
{
    mTilesetDocument->wangSetModel()->setWangSetColorCount(mWangSet, mNewValue);

    QUndoCommand::redo();
}


RemoveWangSetColor::RemoveWangSetColor(TilesetDocument *tilesetDocumnet, WangSet *wangSet, int color)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Remove Wang Color"))
    , mTilesetDocument(tilesetDocumnet)
    , mWangSet(wangSet)
    , mColor(color)
{
    mRemovedWangColor = wangSet->colorAt(mColor);

    const QList<Tile *> changedTiles = wangSet->tilesChangedOnRemoveColor(mColor);

    if (!changedTiles.isEmpty()) {
        QVector<ChangeTileWangId::WangIdChange> changes;

        for (Tile *tile : changedTiles) {
            WangId oldWangId = wangSet->wangIdOfTile(tile);
            WangId changedWangId = oldWangId;

            for (int i = 0; i < WangId::NumIndexes; ++i) {
                const int color = changedWangId.indexColor(i);
                if (color == mColor)
                    changedWangId.setIndexColor(i, 0);
                else if (color > mColor)
                    changedWangId.setIndexColor(i, color - 1);
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
    mTilesetDocument->wangSetModel()->insertWangColor(mWangSet, mRemovedWangColor);

    QUndoCommand::undo();
}

void RemoveWangSetColor::redo()
{
    mTilesetDocument->wangSetModel()->removeWangColorAt(mWangSet, mColor);

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
