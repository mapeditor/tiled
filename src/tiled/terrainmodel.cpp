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

#include "containerhelpers.h"
#include "mapdocument.h"
#include "map.h"
#include "renameterrain.h"
#include "terrain.h"
#include "tile.h"
#include "tilesetdocument.h"
#include "tilesetdocumentsmodel.h"
#include "tileset.h"
#include "tilesetterrainmodel.h"

#include <QApplication>
#include <QFont>
#include <QPalette>

using namespace Tiled;
using namespace Tiled::Internal;

TerrainModel::TerrainModel(QAbstractItemModel *tilesetDocumentsModel,
                           QObject *parent):
    QAbstractItemModel(parent),
    mTilesetDocumentsModel(tilesetDocumentsModel)
{
    connect(mTilesetDocumentsModel, &QAbstractItemModel::rowsInserted,
            this, &TerrainModel::onTilesetRowsInserted);
    connect(mTilesetDocumentsModel, &QAbstractItemModel::rowsAboutToBeRemoved,
            this, &TerrainModel::onTilesetRowsAboutToBeRemoved);
    connect(mTilesetDocumentsModel, &QAbstractItemModel::rowsMoved,
            this, &TerrainModel::onTilesetRowsMoved);
    connect(mTilesetDocumentsModel, &QAbstractItemModel::layoutChanged,
            this, &TerrainModel::onTilesetLayoutChanged);
    connect(mTilesetDocumentsModel, &QAbstractItemModel::dataChanged,
            this, &TerrainModel::onTilesetDataChanged);
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
    for (int row = 0; row < mTilesetDocuments.size(); ++row)
        if (mTilesetDocuments.at(row)->tileset() == tileset)
            return createIndex(row, 0);

    return QModelIndex();
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
        return mTilesetDocuments.size();
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

Qt::ItemFlags TerrainModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

    // Don't allow selecting the tileset headers in the Terrains view
    if (tilesetAt(index))
        defaultFlags &= ~Qt::ItemIsSelectable;

    return defaultFlags;
}

Tileset *TerrainModel::tilesetAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;
    if (index.parent().isValid()) // tilesets don't have parents
        return nullptr;
    if (index.row() >= mTilesetDocuments.size())
        return nullptr;

    return mTilesetDocuments.at(index.row())->tileset().data();
}

Terrain *TerrainModel::terrainAt(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    if (Tileset *tileset = static_cast<Tileset*>(index.internalPointer()))
        return tileset->terrain(index.row());

    return nullptr;
}

void TerrainModel::onTilesetRowsInserted(const QModelIndex &parent, int first, int last)
{
    beginInsertRows(QModelIndex(), first, last);
    for (int row = first; row <= last; ++row) {
        const QModelIndex index = mTilesetDocumentsModel->index(row, 0, parent);
        const QVariant var = mTilesetDocumentsModel->data(index, TilesetDocumentsModel::TilesetDocumentRole);
        TilesetDocument *tilesetDocument = var.value<TilesetDocument*>();

        mTilesetDocuments.insert(row, tilesetDocument);

        TilesetTerrainModel *tilesetTerrainModel = tilesetDocument->terrainModel();
        connect(tilesetTerrainModel, &TilesetTerrainModel::terrainAboutToBeAdded,
                this, &TerrainModel::onTerrainAboutToBeAdded);
        connect(tilesetTerrainModel, &TilesetTerrainModel::terrainAdded,
                this, &TerrainModel::onTerrainAdded);
        connect(tilesetTerrainModel, &TilesetTerrainModel::terrainAboutToBeRemoved,
                this, &TerrainModel::onTerrainAboutToBeRemoved);
        connect(tilesetTerrainModel, &TilesetTerrainModel::terrainRemoved,
                this, &TerrainModel::onTerrainRemoved);
        connect(tilesetTerrainModel, &TilesetTerrainModel::terrainAboutToBeSwapped,
                this, &TerrainModel::onTerrainAboutToBeSwapped);
        connect(tilesetTerrainModel, &TilesetTerrainModel::terrainSwapped,
                this, &TerrainModel::onTerrainSwapped);
    }
    endInsertRows();
}

void TerrainModel::onTilesetRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)

    beginRemoveRows(QModelIndex(), first, last);
    for (int index = last; index >= first; --index) {
        TilesetDocument *tilesetDocument = mTilesetDocuments.takeAt(index);
        tilesetDocument->terrainModel()->disconnect(this);
    }
    endRemoveRows();
}

void TerrainModel::onTilesetRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row)
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

void TerrainModel::onTilesetLayoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint)
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

void TerrainModel::onTilesetDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    emit dataChanged(index(topLeft.row(), topLeft.column()),
                     index(bottomRight.row(), bottomRight.column()));
}

void TerrainModel::onTerrainAboutToBeAdded(Tileset *tileset, int terrainId)
{
    QModelIndex parent = index(tileset);
    beginInsertRows(parent, terrainId, terrainId);
}

void TerrainModel::onTerrainAdded(Tileset *tileset)
{
    endInsertRows();

    // for the TerrainFilterModel
    const QModelIndex index = TerrainModel::index(tileset);
    emit dataChanged(index, index);
}

void TerrainModel::onTerrainAboutToBeRemoved(Terrain *terrain)
{
    QModelIndex parent = index(terrain->tileset());
    beginRemoveRows(parent, terrain->id(), terrain->id());
}

void TerrainModel::onTerrainRemoved(Terrain *terrain)
{
    endRemoveRows();

    // for the TerrainFilterModel
    const QModelIndex index = TerrainModel::index(terrain->tileset());
    emit dataChanged(index, index);
}

void TerrainModel::onTerrainAboutToBeSwapped(Tileset *tileset, int terrainId, int swapTerrainId)
{
    QModelIndex parent = index(tileset);
    beginMoveRows(parent, terrainId, terrainId, parent, swapTerrainId);
}

void TerrainModel::onTerrainSwapped()
{
    endMoveRows();
}
