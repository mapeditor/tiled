/*
 * addremovewangset.cpp
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

#include "addremovewangset.h"

#include "wangset.h"
#include "tileset.h"
#include "tilesetdocument.h"
#include "tilesetwangsetmodel.h"

#include <QCoreApplication>

using namespace Tiled;

AddRemoveWangSet::AddRemoveWangSet(TilesetDocument *tilesetDocument,
                                   int index,
                                   WangSet *wangSet)
    : mTilesetDocument(tilesetDocument)
    , mIndex(index)
    , mWangSet(wangSet)
{
}

AddRemoveWangSet::~AddRemoveWangSet()
{
    delete mWangSet;
}

void AddRemoveWangSet::removeWangSet()
{
    Q_ASSERT(!mWangSet);
    mWangSet = mTilesetDocument->wangSetModel()->takeWangSetAt(mIndex);
}

void AddRemoveWangSet::addWangSet()
{
    Q_ASSERT(mWangSet);
    mTilesetDocument->wangSetModel()->insertWangSet(mIndex, mWangSet);
    mWangSet = nullptr;
}

AddWangSet::AddWangSet(TilesetDocument *tilesetDocument, WangSet *wangSet)
    : AddRemoveWangSet(tilesetDocument,
                       wangSet->tileset()->wangSetCount(),
                       wangSet)
{
    setText(QCoreApplication::translate("Undo Commands", "Add Wang Set"));
}

RemoveWangSet::RemoveWangSet(TilesetDocument *tilesetDocument, WangSet *wangset)
    : AddRemoveWangSet(tilesetDocument,
                       tilesetDocument->wangSetModel()->index(wangset).row(),
                       nullptr)
{
    setText(QCoreApplication::translate("Undo Commands", "Remove Wang Set"));
}
