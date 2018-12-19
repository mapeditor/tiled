/*
 * wangsetmodel.cpp
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

#include "wangsetmodel.h"

#include "containerhelpers.h"
#include "map.h"
#include "mapdocument.h"
#include "wangset.h"
#include "tileset.h"
#include "tile.h"
#include "tilesetdocument.h"
#include "tilesetdocumentsmodel.h"
#include "tileset.h"
#include "tilesetwangsetmodel.h"

#include <QApplication>
#include <QFont>
#include <QPalette>

using namespace Tiled;

WangSetModel::WangSetModel(QAbstractItemModel *tilesetDocumentModel,
                           QObject *parent):
    QAbstractItemModel(parent),
    mTilesetDocumentsModel(tilesetDocumentModel)
{
    connect(mTilesetDocumentsModel, &QAbstractItemModel::rowsInserted,
            this, &WangSetModel::onTilesetRowsInserted);
    connect(mTilesetDocumentsModel, &QAbstractItemModel::rowsAboutToBeRemoved,
            this, &WangSetModel::onTilesetRowsAboutToBeRemoved);
    connect(mTilesetDocumentsModel, &QAbstractItemModel::rowsMoved,
            this, &WangSetModel::onTilesetRowsMoved);
    connect(mTilesetDocumentsModel, &QAbstractItemModel::layoutChanged,
            this, &WangSetModel::onTilesetLayoutChanged);
    connect(mTilesetDocumentsModel, &QAbstractItemModel::dataChanged,
            this, &WangSetModel::onTilesetDataChanged);
}

WangSetModel::~WangSetModel()
{
}

QModelIndex WangSetModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, column);
    else if (Tileset *tileset = tilesetAt(parent))
        return createIndex(row, column, tileset);

    return QModelIndex();
}

QModelIndex WangSetModel::index(Tileset *tileset) const
{
    for (int row = 0; row < mTilesetDocuments.size(); ++row)
        if (mTilesetDocuments.at(row)->tileset() == tileset)
            return createIndex(row, 0);

    return QModelIndex();
}

QModelIndex WangSetModel::index(WangSet *wangSet) const
{
    Tileset *tileset = wangSet->tileset();
    int row = tileset->wangSets().indexOf(wangSet);
    return createIndex(row, 0, tileset);
}

QModelIndex WangSetModel::parent(const QModelIndex &child) const
{
    if (WangSet *wangSet = wangSetAt(child))
        return index(wangSet->tileset());

    return QModelIndex();
}

int WangSetModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return mTilesetDocuments.size();
    else if (Tileset *tileset = tilesetAt(parent))
        return tileset->wangSetCount();

    return 0;
}

int WangSetModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant WangSetModel::data(const QModelIndex &index, int role) const
{
    if (WangSet *wangSet = wangSetAt(index)) {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return wangSet->name();
        case Qt::DecorationRole:
            if (Tile *tile = wangSet->imageTile())
                return tile->image();
            break;
        case WangSetRole:
            return QVariant::fromValue(wangSet);
        }
    } else if (Tileset *tileset = tilesetAt(index)) {
        switch (role) {
        case Qt::DisplayRole:
            return tileset->name();
        case Qt::SizeHintRole:
            return QSize(1, 32);
        case Qt::FontRole: {
            QFont font = QApplication::font();
            font.setBold(true);
            return font;
        }
        case Qt::BackgroundRole: {
            QColor bg = QApplication::palette().alternateBase().color();
            return bg;//.darker(103);
        }
        }
    }

    return QVariant();
}

Qt::ItemFlags WangSetModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    if (tilesetAt(index))
        defaultFlags &= ~Qt::ItemIsSelectable;

    return defaultFlags;
}

Tileset *WangSetModel::tilesetAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;
    if (index.parent().isValid()) // tilesets don't have parents
        return nullptr;
    if (index.row() >= mTilesetDocuments.size())
        return nullptr;

    return mTilesetDocuments.at(index.row())->tileset().data();
}

WangSet *WangSetModel::wangSetAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    if (Tileset *tileset = static_cast<Tileset*>(index.internalPointer()))
        return tileset->wangSet(index.row());

    return nullptr;
}

void WangSetModel::onTilesetRowsInserted(const QModelIndex &parent, int first, int last)
{
    beginInsertRows(QModelIndex(), first, last);
    for (int row = first; row <= last; ++row) {
        const QModelIndex index = mTilesetDocumentsModel->index(row, 0, parent);
        const QVariant var = mTilesetDocumentsModel->data(index, TilesetDocumentsModel::TilesetDocumentRole);
        TilesetDocument *tilesetDocument = var.value<TilesetDocument*>();

        mTilesetDocuments.insert(row, tilesetDocument);

        TilesetWangSetModel *tilesetWangSetModel = tilesetDocument->wangSetModel();
        connect(tilesetWangSetModel, &TilesetWangSetModel::wangSetAboutToBeAdded,
                this, &WangSetModel::onWangSetAboutToBeAdded);
        connect(tilesetWangSetModel, &TilesetWangSetModel::wangSetAdded,
                this, &WangSetModel::onWangSetAdded);
        connect(tilesetWangSetModel, &TilesetWangSetModel::wangSetAboutToBeRemoved,
                this, &WangSetModel::onWangSetAboutToBeRemoved);
        connect(tilesetWangSetModel, &TilesetWangSetModel::wangSetRemoved,
                this, &WangSetModel::onWangSetRemoved);
    }
    endInsertRows();
}

void WangSetModel::onTilesetRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)

    beginRemoveRows(QModelIndex(), first, last);
    for (int index = last; index >= first; --index) {
        TilesetDocument *tilesetDocument = mTilesetDocuments.takeAt(index);
        tilesetDocument->wangSetModel()->disconnect(this);
    }
    endRemoveRows();
}

void WangSetModel::onTilesetRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row)
{
    Q_UNUSED(parent)
    Q_UNUSED(destination)

    beginMoveRows(QModelIndex(), start, end, QModelIndex(), row);

    if (start == row)
        return;

    while (start <= end) {
        mTilesetDocuments.move(start, row);

        if (row < start) {
            ++start;
            ++row;
        } else {
            --end;
        }
    }

    endMoveRows();
}

void WangSetModel::onTilesetLayoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint)
{
    Q_UNUSED(parents)
    Q_UNUSED(hint)

    // Make sure the tileset documents are still in the right order
    for (int i = 0, rows = mTilesetDocuments.size(); i < rows; ++i) {
        const QModelIndex index = mTilesetDocumentsModel->index(i, 0);
        const QVariant var = mTilesetDocumentsModel->data(index, TilesetDocumentsModel::TilesetDocumentRole);
        TilesetDocument *tilesetDocument = var.value<TilesetDocument*>();
        int currentIndex = mTilesetDocuments.indexOf(tilesetDocument);
        if (currentIndex != i) {
            Q_ASSERT(currentIndex > i);
            onTilesetRowsMoved(QModelIndex(), currentIndex, currentIndex, QModelIndex(), i);
        }
    }
}

void WangSetModel::onTilesetDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    emit dataChanged(index(topLeft.row(), topLeft.column()),
                     index(bottomRight.row(), bottomRight.column()));
}

void WangSetModel::onWangSetAboutToBeAdded(Tileset *tileset)
{
    QModelIndex parent = index(tileset);
    beginInsertRows(parent, tileset->wangSetCount(), tileset->wangSetCount());
}

void WangSetModel::onWangSetAdded(Tileset *tileset)
{
    endInsertRows();

    const QModelIndex index = WangSetModel::index(tileset);
    emit dataChanged(index, index);
}

void WangSetModel::onWangSetAboutToBeRemoved(WangSet *wangSet)
{
    QModelIndex parent = index(wangSet->tileset());
    beginRemoveRows(parent, index(wangSet).row(), index(wangSet).row());
}

void WangSetModel::onWangSetRemoved(WangSet *wangSet)
{
    endRemoveRows();

    const QModelIndex index = WangSetModel::index(wangSet->tileset());
    emit dataChanged(index, index);
}
