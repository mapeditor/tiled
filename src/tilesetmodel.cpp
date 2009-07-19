/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
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
    return parent.isValid() ? 0 : (mTileset ? mTileset->tileCount() : 0);
}

int TilesetModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}

QVariant TilesetModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
        return mTileset->tileAt(index.row())->image();

    return QVariant();
}

Tile *TilesetModel::tileAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return mTileset->tileAt(index.row());
}

void TilesetModel::setTileset(Tileset *tileset)
{
    if (mTileset == tileset)
        return;
    mTileset = tileset;
    reset();
}
