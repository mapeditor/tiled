/*
 * tilesetterrainmodel.cpp
 * Copyright 2016, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tilesetterrainmodel.h"

#include "tilesetdocument.h"
#include "renameterrain.h"
#include "terrain.h"
#include "tileset.h"
#include "tile.h"

using namespace Tiled;
using namespace Tiled::Internal;

TilesetTerrainModel::TilesetTerrainModel(TilesetDocument *tilesetDocument,
                                         QObject *parent):
    QAbstractListModel(parent),
    mTilesetDocument(tilesetDocument)
{
}

TilesetTerrainModel::~TilesetTerrainModel()
{
}

QModelIndex TilesetTerrainModel::index(Terrain *terrain) const
{
    Tileset *tileset = terrain->tileset();
    int row = tileset->terrains().indexOf(terrain);
    return index(row, 0);
}

int TilesetTerrainModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return mTilesetDocument->tileset()->terrainCount();

    return 0;
}

int TilesetTerrainModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant TilesetTerrainModel::data(const QModelIndex &index, int role) const
{
    if (Terrain *terrain = terrainAt(index)) {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return terrain->name();
        case Qt::DecorationRole:
            if (Tile *imageTile = terrain->imageTile())
                return imageTile->image();
            break;
        case TerrainRole:
            return QVariant::fromValue(terrain);
        }
    }

    return QVariant();
}

bool TilesetTerrainModel::setData(const QModelIndex &index,
                                  const QVariant &value,
                                  int role)
{
    if (role == Qt::EditRole) {
        const QString newName = value.toString();
        Terrain *terrain = terrainAt(index);
        if (terrain->name() != newName) {
            RenameTerrain *rename = new RenameTerrain(mTilesetDocument,
                                                      terrain->id(),
                                                      newName);
            mTilesetDocument->undoStack()->push(rename);
        }
        return true;
    }

    return false;
}

Qt::ItemFlags TilesetTerrainModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags rc = QAbstractItemModel::flags(index);
    if (index.isValid())  // can edit terrain names
        rc |= Qt::ItemIsEditable;
    return rc;
}

Terrain *TilesetTerrainModel::terrainAt(const QModelIndex &index) const
{
    if (index.isValid())
        return mTilesetDocument->tileset()->terrain(index.row());

    return nullptr;
}

/**
 * Adds a terrain type at \a index. Emits the appropriate signal.
 */
void TilesetTerrainModel::insertTerrain(int index, Terrain *terrain)
{
    Tileset *tileset = mTilesetDocument->tileset().data();

    emit terrainAboutToBeAdded(tileset, index);

    beginInsertRows(QModelIndex(), index, index);
    tileset->insertTerrain(index, terrain);
    endInsertRows();

    emit terrainAdded(tileset, index);
}

/**
 * Removes the terrain type from the given \a tileset at \a index and returns
 * it. The caller becomes responsible for the lifetime of the terrain type.
 * Emits the appropriate signal.
 *
 * \warning This will update terrain information of all the tiles in the
 *          tileset, clearing references to the removed terrain.
 */
Terrain *TilesetTerrainModel::takeTerrainAt(int index)
{
    Tileset *tileset = mTilesetDocument->tileset().data();

    emit terrainAboutToBeRemoved(tileset->terrain(index));

    beginRemoveRows(QModelIndex(), index, index);
    Terrain *terrain = tileset->takeTerrainAt(index);
    endRemoveRows();

    emit terrainRemoved(terrain);

    return terrain;
}

/**
 * Swaps a terrain type at \a index with another index.
 */
void TilesetTerrainModel::swapTerrains(int index, int swapIndex)
{
    Q_ASSERT(index != swapIndex);

    Tileset *tileset = mTilesetDocument->tileset().data();
    int swapIndexChild = swapIndex + (swapIndex > index ? 1 : 0);

    emit terrainAboutToBeSwapped(tileset, index, swapIndexChild);

    beginMoveRows(QModelIndex(), index, index, QModelIndex(), swapIndexChild);

    tileset->swapTerrains(index, swapIndex);

    emit terrainSwapped(tileset);

    endMoveRows();
}

void TilesetTerrainModel::setTerrainName(int index, const QString &name)
{
    Tileset *tileset = mTilesetDocument->tileset().data();
    Terrain *terrain = tileset->terrain(index);
    terrain->setName(name);
    emitTerrainChanged(terrain);
}

void TilesetTerrainModel::setTerrainImage(int index, int tileId)
{
    Tileset *tileset = mTilesetDocument->tileset().data();
    Terrain *terrain = tileset->terrain(index);
    terrain->setImageTileId(tileId);
    emitTerrainChanged(terrain);
}

void TilesetTerrainModel::emitTerrainChanged(Terrain *terrain)
{
    const QModelIndex index = TilesetTerrainModel::index(terrain);
    emit dataChanged(index, index);
    emit terrainChanged(terrain->tileset(), index.row());
}
