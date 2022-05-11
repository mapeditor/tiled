/*
 * tilesetwangsetmodel.cpp
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

#include "tilesetwangsetmodel.h"

#include "changeevents.h"
#include "changewangsetdata.h"
#include "tile.h"
#include "tileset.h"
#include "tilesetdocument.h"
#include "wangoverlay.h"
#include "wangset.h"

using namespace Tiled;

TilesetWangSetModel::TilesetWangSetModel(TilesetDocument *tilesetDocument,
                                         QObject *parent):
    QAbstractListModel(parent),
    mTilesetDocument(tilesetDocument)
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
    if (!parent.isValid())
        return mTilesetDocument->tileset()->wangSetCount();

    return 0;
}

int TilesetWangSetModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}


QVariant TilesetWangSetModel::data(const QModelIndex &index, int role) const
{
    if (WangSet *wangSet = wangSetAt(index)) {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return wangSet->name();
        case Qt::DecorationRole:
            if (Tile *tile = wangSet->imageTile())
                return tile->image().copy(tile->imageRect());
            else
                return wangSetIcon(wangSet->type());
            break;
        case WangSetRole:
            return QVariant::fromValue(wangSet);
        case TilesetDocumentRole:
            return QVariant::fromValue(mTilesetDocument);
        }
    }

    return QVariant();
}

bool TilesetWangSetModel::setData(const QModelIndex &index,
                                  const QVariant &value,
                                  int role)
{
    if (role == Qt::EditRole) {
        const QString newName = value.toString();
        WangSet *wangSet = wangSetAt(index);
        if (wangSet->name() != newName) {
            RenameWangSet *rename = new RenameWangSet(mTilesetDocument,
                                                      wangSet,
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

void TilesetWangSetModel::insertWangSet(int index, std::unique_ptr<WangSet> wangSet)
{
    Tileset *tileset = mTilesetDocument->tileset().data();

    emit mTilesetDocument->changed(WangSetEvent(ChangeEvent::WangSetAboutToBeAdded, tileset, index));

    beginInsertRows(QModelIndex(), index, index);
    tileset->insertWangSet(index, std::move(wangSet));
    endInsertRows();

    emit mTilesetDocument->changed(WangSetEvent(ChangeEvent::WangSetAdded, tileset, index));
    emit wangSetAdded(tileset, index);
}

std::unique_ptr<WangSet> TilesetWangSetModel::takeWangSetAt(int index)
{
    Tileset *tileset = mTilesetDocument->tileset().data();

    emit mTilesetDocument->changed(WangSetEvent(ChangeEvent::WangSetAboutToBeRemoved, tileset, index));

    beginRemoveRows(QModelIndex(), index, index);
    std::unique_ptr<WangSet> wangSet = tileset->takeWangSetAt(index);
    endRemoveRows();

    emit mTilesetDocument->changed(WangSetEvent(ChangeEvent::WangSetRemoved, tileset, index));
    emit wangSetRemoved(wangSet.get());

    return wangSet;
}

void TilesetWangSetModel::setWangSetName(WangSet *wangSet, const QString &name)
{
    Q_ASSERT(wangSet->tileset() == mTilesetDocument->tileset().data());
    wangSet->setName(name);
    emitWangSetChange(wangSet);
}

void TilesetWangSetModel::setWangSetType(WangSet *wangSet, WangSet::Type type)
{
    Q_ASSERT(wangSet->tileset() == mTilesetDocument->tileset().data());
    wangSet->setType(type);
    emit mTilesetDocument->changed(WangSetChangeEvent(wangSet, WangSetChangeEvent::TypeProperty));
}

void TilesetWangSetModel::setWangSetColorCount(WangSet *wangSet, int value)
{
    Q_ASSERT(wangSet->tileset() == mTilesetDocument->tileset().data());
    wangSet->setColorCount(value);
    emitWangSetChange(wangSet);
}

void TilesetWangSetModel::setWangSetImage(WangSet *wangSet, int tileId)
{
    Q_ASSERT(wangSet->tileset() == mTilesetDocument->tileset().data());
    wangSet->setImageTileId(tileId);
    emitWangSetChange(wangSet);
}

void TilesetWangSetModel::insertWangColor(WangSet *wangSet, const QSharedPointer<WangColor> &wangColor)
{
    Q_ASSERT(wangSet->tileset() == mTilesetDocument->tileset().data());
    wangSet->insertWangColor(wangColor);
    emitWangSetChange(wangSet);
}

QSharedPointer<WangColor> TilesetWangSetModel::takeWangColorAt(WangSet *wangSet, int color)
{
    Q_ASSERT(wangSet->tileset() == mTilesetDocument->tileset().data());
    emit mTilesetDocument->changed(WangColorEvent(ChangeEvent::WangColorAboutToBeRemoved, wangSet, color));
    auto wangColor = wangSet->takeWangColorAt(color);
    emit wangColorRemoved(wangColor.data());
    emitWangSetChange(wangSet);
    return wangColor;
}

void TilesetWangSetModel::emitWangSetChange(WangSet *wangSet)
{
    const QModelIndex index = TilesetWangSetModel::index(wangSet);
    emit dataChanged(index, index);
    emit wangSetChanged(wangSet);
}

#include "moc_tilesetwangsetmodel.cpp"
