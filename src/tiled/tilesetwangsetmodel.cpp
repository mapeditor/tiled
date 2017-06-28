/*
 * tilesetwangsetmodel.cpp
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

#include "tilesetwangsetmodel.h"

#include "tilesetdocument.h"
#include "renamewangset.h"
#include "wangset.h"
#include "tileset.h"
#include "tile.h"

using namespace Tiled;
using namespace Internal;

TilesetWangSetModel::TilesetWangSetModel(TilesetDocument *mapDocument,
                                         QObject *parent):
    QAbstractListModel(parent),
    mTilesetDocument(mapDocument)
{
}

TilesetWangSetModel::~TilesetWangSetModel()
{
}

QModelIndex TilesetWangSetModel::index(WangSet *wangSet)
{
    Tileset *tileset = wangSet->tileset();
    int row = tileset->wangSets().indexOf(wangSet);

    return index(row, 0);
}

int TilesetWangSetModel::rowCount(const QModelIndex &parent) const
{
    if(!parent.isValid())
        return mTilesetDocument->tileset()->wangSetCount();

    return 0;
}

int TilesetWangSetModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}


QVariant TilesetWangSetModel::data(const QModelIndex &index, int role) const
{
    if(WangSet *wangSet = wangSetAt(index)) {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return wangSet->name();
        case Qt::DecorationRole:
            if (Tile *imageTile = wangSet->imageTile())
                return imageTile->image();
            break;
        case WangSetRole:
            return QVariant::fromValue(wangSet);
        }
    }

    return QVariant();
}

bool TilesetWangSetModel::setData(const QModelIndex &index,
                                  const QVariant &value,
                                  int role)
{
    if(role == Qt::EditRole) {
        const QString newName = value.toString();
        WangSet *wangSet = wangSetAt(index);
        if (wangSet->name() != newName) {
            RenameWangSet *rename = new RenameWangSet(mTilesetDocument,
                                                      mTilesetDocument->tileset()->wangSets().indexOf(wangSet),
                                                      newName);
            mTilesetDocument->undoStack()->push(rename);
        }
        return true;
    }

    return false;
}

Qt::ItemFlags TilesetWangSetModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags rc = QAbstractItemModel::flags(index);
    if (index.isValid())  // can edit wangSet names
        rc |= Qt::ItemIsEditable;
    return rc;
}

WangSet *TilesetWangSetModel::wangSetAt(const QModelIndex &index) const
{
    if (index.isValid())
        return mTilesetDocument->tileset()->wangSet(index.row());

    return nullptr;
}

void TilesetWangSetModel::insertWangSet(WangSet *wangSet)
{
    Tileset *tileset = mTilesetDocument->tileset().data();
    int row = tileset->wangSetCount();

    emit wangSetAboutToBeAdded(tileset);

    beginInsertRows(QModelIndex(), row, row);
    tileset->insertWangSet(wangSet);
    endInsertRows();

    emit wangSetAdded(tileset);
}

WangSet *TilesetWangSetModel::takeWangSetAt(int index)
{
    Tileset *tileset = mTilesetDocument->tileset().data();

    emit wangSetAboutToBeRemoved(tileset->wangSet(index));

    beginRemoveRows(QModelIndex(), index, index);
    WangSet *wangSet = tileset->takeWangSetAt(index);
    endRemoveRows();

    emit wangSetRemoved(wangSet);

    return wangSet;
}

void TilesetWangSetModel::setWangSetName(int index, const QString &name)
{
    Tileset *tileset = mTilesetDocument->tileset().data();
    WangSet *wangSet = tileset->wangSet(index);
    wangSet->setName(name);
    emitWangSetChange(wangSet);
}

void TilesetWangSetModel::setWangSetImage(int index, int tileId)
{
    Tileset *tileset = mTilesetDocument->tileset().data();
    WangSet *wangSet = tileset->wangSet(index);
    wangSet->setImageTileId(tileId);
    emitWangSetChange(wangSet);
}

void TilesetWangSetModel::emitWangSetChange(WangSet *wangSet)
{
    const QModelIndex index = TilesetWangSetModel::index(wangSet);
    emit dataChanged(index, index);
    emit wangSetChanged(wangSet, index.row());
}
