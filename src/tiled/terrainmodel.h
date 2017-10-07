/*
 * terrainmodel.h
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

#pragma once

#include <QAbstractItemModel>
#include <tileset.h>

namespace Tiled {

class Tileset;
class Terrain;

namespace Internal {

class TilesetDocument;

/**
 * A model providing a tree view on the terrain types available on a map.
 */
class TerrainModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum UserRoles {
        TerrainRole = Qt::UserRole
    };

    /**
     * Constructor.
     *
     * @param tilesetDocumentsModel a model of a list of tileset documents
     */
    TerrainModel(QAbstractItemModel *tilesetDocumentsModel,
                 QObject *parent = nullptr);

    ~TerrainModel();

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(Tileset *tileset) const;
    QModelIndex index(Terrain *terrain) const;

    QModelIndex parent(const QModelIndex &child) const override;

    /**
     * Returns the number of rows. For the root, this is the number of tilesets
     * with terrain types defined. Otherwise it is the number of terrain types
     * in a certain tileset.
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * Returns the tileset at the given \a index, or 0 if there is no tileset.
     */
    Tileset *tilesetAt(const QModelIndex &index) const;

    /**
     * Returns the terrain at the given \a index, or 0 if there is no terrain.
     */
    Terrain *terrainAt(const QModelIndex &index) const;

private slots:
    void onTilesetRowsInserted(const QModelIndex &parent, int first, int last);
    void onTilesetRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void onTilesetRowsMoved(const QModelIndex &parent, int start, int end, const QModelIndex &destination, int row);
    void onTilesetLayoutChanged(const QList<QPersistentModelIndex> &parents, QAbstractItemModel::LayoutChangeHint hint);
    void onTilesetDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

    void onTerrainAboutToBeAdded(Tileset *tileset, int terrainId);
    void onTerrainAdded(Tileset *tileset);
    void onTerrainAboutToBeRemoved(Terrain *terrain);
    void onTerrainRemoved(Terrain *terrain);
    void onTerrainAboutToBeSwapped(Tileset *tileset, int terrainId, int swapTerrainId);
    void onTerrainSwapped();

private:
    QAbstractItemModel *mTilesetDocumentsModel;
    QList<TilesetDocument*> mTilesetDocuments;
};

} // namespace Internal
} // namespace Tiled
