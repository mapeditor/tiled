/*
 * tilesetmodel.cpp
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
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

#include "tilesetmodel.h"

#include "map.h"
#include "tile.h"
#include "tileset.h"

using namespace Tiled;
using namespace Tiled::Internal;

TilesetModel::TilesetModel(Tileset *tileset, QObject *parent):
    QAbstractListModel(parent),
    mTileset(tileset)
{
}

int TilesetModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    const int tiles = mTileset->tileCount();
    const int columns = mTileset->columnCount();

    int rows = 1;
    if (columns > 0) {
        rows = tiles / columns;
        if (tiles % columns > 0)
            ++rows;
    }

    return rows;
}

int TilesetModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mTileset->columnCount();
}

QVariant TilesetModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        if (Tile *tile = tileAt(index))
            return tile->image();
    }

    return QVariant();
}

QVariant TilesetModel::headerData(int /* section */,
                                  Qt::Orientation /* orientation */,
                                  int role) const
{
    if (role == Qt::SizeHintRole)
        return QSize(1, 1);
    return QVariant();
}

Tile *TilesetModel::tileAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    const int i = index.column() + index.row() * mTileset->columnCount();
    return mTileset->tileAt(i);
}

void TilesetModel::setTileset(Tileset *tileset)
{
    if (mTileset == tileset)
        return;
    mTileset = tileset;
    reset();
}
