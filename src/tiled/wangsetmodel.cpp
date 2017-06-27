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

#include <QApplication>
#include <QFont>
#include <QPalette>

using namespace Tiled;
using namespace Tiled::Internal;

WangSetModel::WangSetModel(MapDocument *mapDocument,
                           QObject *parent):
    QAbstractItemModel(parent),
    mMapDocument(mapDocument)
{
    connect(mapDocument, SIGNAL(tilesetAboutToBeAdded(int)),
            this, SLOT(tilesetAboutToBeAdded(int)));
    connect(mapDocument, SIGNAL(tilesetAdded(int,Tileset*)),
            this, SLOT(tilesetAdded()));
    connect(mapDocument, SIGNAL(tilesetAboutToBeRemoved(int)),
            this, SLOT(tilesetAboutToBeRemoved(int)));
    connect(mapDocument, SIGNAL(tilesetRemoved(Tileset*)),
            this, SLOT(tilesetRemoved()));
    connect(mapDocument, &MapDocument::tilesetNameChanged,
            this, &WangSetModel::tilesetChanged);

    connect(mapDocument, &MapDocument::tilesetWangSetAboutToBeAdded,
            this, &WangSetModel::wangSetAboutToBeAdded);
    connect(mapDocument, &MapDocument::tilesetWangSetAdded,
            this, &WangSetModel::wangSetAdded);
    connect(mapDocument, &MapDocument::tilesetWangSetAboutToBeRemoved,
            this, &WangSetModel::wangSetAboutToBeRemoved);
    connect(mapDocument, &MapDocument::tilesetWangSetRemoved,
            this, &WangSetModel::wangSetRemoved);
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

QModelIndex WangSetModel::index(Tileset *tileSet) const
{
    int row = indexOf(mMapDocument->map()->tilesets(), tileSet);
    Q_ASSERT(row != -1);
    return createIndex(row, 0);
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
        return mMapDocument->map()->tilesetCount();
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

Tileset *WangSetModel::tilesetAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;
    if (index.parent().isValid()) // tilesets don't have parents
        return nullptr;
    if (index.row() >= mMapDocument->map()->tilesetCount())
        return nullptr;

    return mMapDocument->map()->tilesetAt(index.row()).data();
}

WangSet *WangSetModel::wangSetAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    if (Tileset *tileset = static_cast<Tileset*>(index.internalPointer()))
        return tileset->wangSet(index.row());

    return nullptr;
}

void WangSetModel::tilesetAboutToBeAdded(int index)
{
    beginInsertRows(QModelIndex(), index, index);
}

void WangSetModel::tilesetAdded()
{
    endInsertRows();
}

void WangSetModel::tilesetAboutToBeRemoved(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
}

void WangSetModel::tilesetRemoved()
{
    endRemoveRows();
}

void WangSetModel::tilesetChanged(Tileset *tileset)
{
    const QModelIndex index = WangSetModel::index(tileset);
    emit dataChanged(index, index);
}

void WangSetModel::wangSetAboutToBeAdded(Tileset *tileset)
{
    QModelIndex parent = index(tileset);
    beginInsertRows(parent, tileset->wangSetCount(), tileset->wangSetCount());
}

void WangSetModel::wangSetAdded(Tileset *tileset)
{
    endInsertRows();
    tilesetChanged(tileset);
}

void WangSetModel::wangSetAboutToBeRemoved(Tileset *tileset, WangSet *wangSet)
{
    QModelIndex parent = index(tileset);
    beginRemoveRows(parent, tileset->wangSets().indexOf(wangSet), tileset->wangSets().indexOf(wangSet));
}

void WangSetModel::wangSetRemoved(Tileset *tileset)
{
    endRemoveRows();
    tilesetChanged(tileset);
}
