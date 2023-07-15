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

#include "changeevents.h"
#include "changetilewangid.h"
#include "tileset.h"
#include "tilesetdocument.h"
#include "tilesetwangsetmodel.h"

#include <QCoreApplication>

using namespace Tiled;

RenameWangSet::RenameWangSet(TilesetDocument *tilesetDocument,
                             WangSet *wangSet,
                             const QString &newName)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Terrain Set Name"))
    , mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
    , mOldName(wangSet->name())
    , mNewName(newName)
{
}

void RenameWangSet::undo()
{
    mTilesetDocument->wangSetModel()->setWangSetName(mWangSet, mOldName);
}

void RenameWangSet::redo()
{
    mTilesetDocument->wangSetModel()->setWangSetName(mWangSet, mNewName);
}

bool RenameWangSet::mergeWith(const QUndoCommand *other)
{
    auto o = static_cast<const RenameWangSet*>(other);
    if (mWangSet != o->mWangSet)
        return false;

    mNewName = o->mNewName;
    return true;
}


ChangeWangSetType::ChangeWangSetType(TilesetDocument *tilesetDocument,
                                     WangSet *wangSet,
                                     WangSet::Type newType,
                                     QUndoCommand *parent)
    : QUndoCommand(parent)
    , mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
    , mOldType(wangSet->type())
    , mNewType(newType)
{
    setText(QCoreApplication::translate("Undo Commands", "Change Terrain Set Type"));
}

void ChangeWangSetType::undo()
{
    mTilesetDocument->wangSetModel()->setWangSetType(mWangSet, mOldType);
}

void ChangeWangSetType::redo()
{
    mTilesetDocument->wangSetModel()->setWangSetType(mWangSet, mNewType);
}


ChangeWangSetColorCount::ChangeWangSetColorCount(TilesetDocument *tilesetDocument,
                                                 WangSet *wangSet,
                                                 int newValue)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Terrain Count"))
    , mTilesetDocument(tilesetDocument)
    , mWangSet(wangSet)
    , mOldValue(wangSet->colorCount())
    , mNewValue(newValue)
{
    // when edge size changes, all tiles with WangIds need to be updated.
    if (mNewValue < mOldValue) {
        // when the size is reduced, some Wang assignments can be lost.
        const auto changes = ChangeTileWangId::changesOnSetColorCount(wangSet, mNewValue);
        if (!changes.isEmpty())
            new ChangeTileWangId(mTilesetDocument, wangSet, changes, this);

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

    for (const WangColorChange &w : std::as_const(mRemovedWangColors)) {
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
                                               "Remove Terrain"))
    , mTilesetDocument(tilesetDocumnet)
    , mWangSet(wangSet)
    , mColor(color)
{
    const auto changes = ChangeTileWangId::changesOnRemoveColor(wangSet, color);
    if (!changes.isEmpty())
        new ChangeTileWangId(mTilesetDocument, wangSet, changes, this);
}

void RemoveWangSetColor::undo()
{
    mTilesetDocument->wangSetModel()->insertWangColor(mWangSet, std::move(mRemovedWangColor));
    QUndoCommand::undo();
}

void RemoveWangSetColor::redo()
{
    mRemovedWangColor = mTilesetDocument->wangSetModel()->takeWangColorAt(mWangSet, mColor);
    QUndoCommand::redo();
}


SetWangSetImage::SetWangSetImage(TilesetDocument *tilesetDocument,
                                 WangSet *wangSet,
                                 int tileId,
                                 QUndoCommand *parent)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Set Terrain Set Image"),
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
