/*
 * tilesetterrainmodel.h
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

#pragma once

#include <QAbstractItemModel>

namespace Tiled {

class Tileset;
class Terrain;

namespace Internal {

class TilesetDocument;

/**
 * A model providing a tree view on the terrain types available on a map.
 */
class TilesetTerrainModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum UserRoles {
        TerrainRole = Qt::UserRole
    };

    /**
     * Constructor.
     *
     * @param mapDocument the map to manage terrains for
     */
    TilesetTerrainModel(TilesetDocument *mapDocument,
                        QObject *parent = nullptr);

    ~TilesetTerrainModel();

    using QAbstractListModel::index;
    QModelIndex index(Terrain *terrain) const;

    /**
     * Returns the number of rows. For the root, this is the number of terrain
     * types in the tileset.
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * Returns the number of columns.
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    /**
     * Returns the data stored under the given <i>role</i> for the item
     * referred to by the <i>index</i>.
     */
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    /**
     * Allows for changing the name of a terrain.
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;

    /**
     * Makes terrain names editable.
     */
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * Returns the terrain at the given \a index, or 0 if there is no terrain.
     */
    Terrain *terrainAt(const QModelIndex &index) const;

    void insertTerrain(int index, Terrain *terrain);
    Terrain *takeTerrainAt(int index);
    void swapTerrains(int index, int swapIndex);
    void setTerrainName(int index, const QString &name);
    void setTerrainImage(int index, int tileId);

signals:
    void terrainAboutToBeAdded(Tileset *tileset, int terrainId);
    void terrainAdded(Tileset *tileset, int terrainId);
    void terrainAboutToBeRemoved(Terrain *terrain);
    void terrainRemoved(Terrain *terrain);
    void terrainAboutToBeSwapped(Tileset *tileset, int terrainId, int swapTerrainId);
    void terrainSwapped(Tileset *tileset);

    /**
     * Emitted when either the name or the image of a terrain changed.
     */
    void terrainChanged(Tileset *tileset, int index);

private:
    void emitTerrainChanged(Terrain *terrain);

    TilesetDocument *mTilesetDocument;
};

} // namespace Internal
} // namespace Tiled
