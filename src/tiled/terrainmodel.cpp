/*
 * terrainmodel.cpp
 * Copyright 2008-2012, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#include "mapdocument.h"
#include "renameterrain.h"
#include "terrain.h"
#include "tileset.h"
#include "tile.h"

#include <QApplication>
#include <QFont>
#include <QPalette>

using namespace Tiled;
using namespace Tiled::Internal;

TerrainModel::TerrainModel(MapDocument *mapDocument,
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
    connect(mapDocument, SIGNAL(tilesetNameChanged(Tileset*)),
            this, SLOT(tilesetNameChanged(Tileset*)));
}

TerrainModel::~TerrainModel()
{
}

QModelIndex TerrainModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, column);
    else if (Tileset *tileset = tilesetAt(parent))
        return createIndex(row, column, tileset);

    return QModelIndex();
}

QModelIndex TerrainModel::index(Tileset *tileset) const
{
    int row = mMapDocument->map()->tilesets().indexOf(tileset);
    Q_ASSERT(row != -1);
    return createIndex(row, 0);
}

QModelIndex TerrainModel::index(Terrain *terrain) const
{
    Tileset *tileset = terrain->tileset();
    int row = tileset->terrains().indexOf(terrain);
    return createIndex(row, 0, tileset);
}

QModelIndex TerrainModel::parent(const QModelIndex &child) const
{
    if (Terrain *terrain = terrainAt(child))
        return index(terrain->tileset());

    return QModelIndex();
}

int TerrainModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return mMapDocument->map()->tilesetCount();
    else if (Tileset *tileset = tilesetAt(parent))
        return tileset->terrainCount();

    return 0;
}

int TerrainModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant TerrainModel::data(const QModelIndex &index, int role) const
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

bool TerrainModel::setData(const QModelIndex &index,
                           const QVariant &value,
                           int role)
{
    if (role == Qt::EditRole) {
        const QString newName = value.toString();
        Terrain *terrain = terrainAt(index);
        if (terrain->name() != newName) {
            RenameTerrain *rename = new RenameTerrain(mMapDocument,
                                                      terrain->tileset(),
                                                      terrain->id(),
                                                      newName);
            mMapDocument->undoStack()->push(rename);
        }
        return true;
    }

    return false;
}

Qt::ItemFlags TerrainModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags rc = QAbstractItemModel::flags(index);
    if (index.parent().isValid())  // can edit terrain names
        rc |= Qt::ItemIsEditable;
    return rc;
}

Tileset *TerrainModel::tilesetAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    if (index.parent().isValid()) // tilesets don't have parents
        return 0;
    if (index.row() >= mMapDocument->map()->tilesetCount())
        return 0;

    return mMapDocument->map()->tilesetAt(index.row());
}

Terrain *TerrainModel::terrainAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    if (Tileset *tileset = static_cast<Tileset*>(index.internalPointer()))
        return tileset->terrain(index.row());

    return 0;
}

/**
 * Adds a terrain type to the given \a tileset at \a index. Emits the
 * appropriate signal.
 */
void TerrainModel::insertTerrain(Tileset *tileset, int index, Terrain *terrain)
{
    const QModelIndex tilesetIndex = TerrainModel::index(tileset);

    beginInsertRows(tilesetIndex, index, index);
    tileset->insertTerrain(index, terrain);
    endInsertRows();
    emit terrainAdded(tileset, index);
    emit dataChanged(tilesetIndex, tilesetIndex); // for TerrainFilterModel
}

/**
 * Removes the terrain type from the given \a tileset at \a index and returns
 * it. The caller becomes responsible for the lifetime of the terrain type.
 * Emits the appropriate signal.
 *
 * \warning This will update terrain information of all the tiles in the
 *          tileset, clearing references to the removed terrain.
 */
Terrain *TerrainModel::takeTerrainAt(Tileset *tileset, int index)
{
    const QModelIndex tilesetIndex = TerrainModel::index(tileset);

    beginRemoveRows(tilesetIndex, index, index);
    Terrain *terrain = tileset->takeTerrainAt(index);
    endRemoveRows();
    emit terrainRemoved(terrain);
    emit dataChanged(tilesetIndex, tilesetIndex); // for TerrainFilterModel

    return terrain;
}

void TerrainModel::setTerrainName(Tileset *tileset, int index, const QString &name)
{
    Terrain *terrain = tileset->terrain(index);
    terrain->setName(name);
    emitTerrainChanged(terrain);
}

void TerrainModel::setTerrainImage(Tileset *tileset, int index, int tileId)
{
    Terrain *terrain = tileset->terrain(index);
    terrain->setImageTileId(tileId);
    emitTerrainChanged(terrain);
}

void TerrainModel::emitTerrainChanged(Terrain *terrain)
{
    const QModelIndex index = TerrainModel::index(terrain);
    emit dataChanged(index, index);
    emit terrainChanged(terrain->tileset(), index.row());
}

void TerrainModel::tilesetAboutToBeAdded(int index)
{
    beginInsertRows(QModelIndex(), index, index);
}

void TerrainModel::tilesetAdded()
{
    endInsertRows();
}

void TerrainModel::tilesetAboutToBeRemoved(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
}

void TerrainModel::tilesetRemoved()
{
    endRemoveRows();
}

void TerrainModel::tilesetNameChanged(Tileset *tileset)
{
    const QModelIndex index = TerrainModel::index(tileset);
    emit dataChanged(index, index);
}
