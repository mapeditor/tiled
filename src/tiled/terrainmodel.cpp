/*
 * terrainmodel.cpp
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Edward Hutchins <eah1@yahoo.com>
 * Copyright 2012, Manu Evans <turkeyman@gmail.com>
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

#include "terrainmodel.h"

#include "map.h"
#include "terrain.h"
#include "tileset.h"
#include "tile.h"

using namespace Tiled;
using namespace Tiled::Internal;

TerrainModel::TerrainModel(Tileset *tileset, QObject *parent):
    QAbstractListModel(parent),
    mTileset(tileset)
{
}

int TerrainModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return mTileset->terrainCount() + 1;
}

int TerrainModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}

QVariant TerrainModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        if (Terrain *terrain = terrainAt(index))
            if (terrain)
                return terrain->paletteImage()->image();
    }

    return QVariant();
}

QVariant TerrainModel::headerData(int /* section */,
                                  Qt::Orientation /* orientation */,
                                  int role) const
{
    if (role == Qt::SizeHintRole)
        return QSize(1, 1);
    return QVariant();
}

Terrain *TerrainModel::terrainAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    const int i = index.column() + index.row() * columnCount() - 1;
    return i == -1 || i >= mTileset->terrainCount() ? NULL : mTileset->terrain(i);
}

void TerrainModel::setTileset(Tileset *tileset)
{
    if (mTileset == tileset)
        return;
    mTileset = tileset;
    reset();
}
