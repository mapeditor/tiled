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

#include <QCoreApplication>

using namespace Tiled;
using namespace Internal;

ChangeWangSetEdges::ChangeWangSetEdges(TilesetDocument *tilesetDocument,
                                       int index,
                                       int newValue)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Wang Set edge count"))
    , mWangSetModel(tilesetDocument->wangSetModel())
    , mIndex(index)
    , mOldValue(tilesetDocument->tileset()->wangSet(index)->edgeColors())
    , mNewValue(newValue)
{
}

void ChangeWangSetEdges::undo()
{
    mWangSetModel->setWangSetEdges(mIndex, mOldValue);
}

void ChangeWangSetEdges::redo()
{
    mWangSetModel->setWangSetEdges(mIndex, mNewValue);
}

ChangeWangSetCorners::ChangeWangSetCorners(TilesetDocument *tilesetDocument,
                                       int index,
                                       int newValue)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Wang Set corner count"))
    , mWangSetModel(tilesetDocument->wangSetModel())
    , mIndex(index)
    , mOldValue(tilesetDocument->tileset()->wangSet(index)->cornerColors())
    , mNewValue(newValue)
{
}

void ChangeWangSetCorners::undo()
{
    mWangSetModel->setWangSetCorners(mIndex, mOldValue);
}

void ChangeWangSetCorners::redo()
{
    mWangSetModel->setWangSetCorners(mIndex, mNewValue);
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
