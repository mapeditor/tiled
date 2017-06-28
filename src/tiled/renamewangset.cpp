/*
 * renamewangset.h
 * Copyright 2017, Benjamin Trotte <bdtrotte@ucsc.edu>
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

#include "renamewangset.h"

#include "wangset.h"
#include "tileset.h"
#include "tilesetdocument.h"
#include "tilesetwangsetmodel.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Internal;

RenameWangSet::RenameWangSet(TilesetDocument *tilesetDocument,
                             int index,
                             const QString &newName)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Wang Set Name"))
    , mWangSetModel(tilesetDocument->wangSetModel())
    , mTileset(tilesetDocument->tileset().data())
    , mIndex(index)
    , mOldName(mTileset->wangSet(index)->name())
    , mNewName(newName)
{
}

void RenameWangSet::undo()
{
    mWangSetModel->setWangSetName(mIndex, mOldName);
}

void RenameWangSet::redo()
{
    mWangSetModel->setWangSetName(mIndex, mNewName);
}
