/*
 * tilesetmodel.h
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

#pragma once

#include "tile.h"

#include <QAbstractListModel>

namespace Tiled {

class Tileset;
class TilesetDocument;

/**
 * A model wrapping a tileset of a map. Used to display the tiles.
 */
class TilesetModel : public QAbstractListModel
{
    Q_OBJECT

public:
    /**
     * Constructor.
     *
     * @param tilesetDocument the initial tileset to display
     */
    TilesetModel(TilesetDocument *tilesetDocument, QObject *parent = nullptr);

    /**
     * Returns the number of rows.
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
     * Returns a small size hint, to prevent the headers from affecting the
     * minimum width and height of the sections.
     */
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    Qt::DropActions supportedDropActions() const override;

    QStringList mimeTypes() const override;
    QMimeData *mimeData(const QModelIndexList &indexes) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action,
                      int row, int column,
                      const QModelIndex &parent) override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

    /**
     * Returns the tile at the given index.
     */
    Tile *tileAt(const QModelIndex &index) const;

    /**
     * Returns the index of the given \a tile. The tile is required to be from
     * the tileset used by this model.
     */
    QModelIndex tileIndex(const Tile *tile) const;

    /**
     * Returns the tileset associated with this model.
     */
    Tileset *tileset() const;

    /**
     * Refreshes the list of tile IDs. Should be called after tiles are added
     * or removed from the tileset.
     */
    void tilesetChanged();

    void setColumnCountOverride(int columnCount);

    bool isFixedAtlas() const;

    QPoint snapToGrid(const QPoint &pos) const;

public slots:
    /**
     * Should be called when anything changes about the given \a tiles that
     * affects their display in any views on this model.
     *
     * Tiles that are not from the tileset displayed by this model are simply
     * ignored. All tiles in the list are assumed to be from the same tileset.
     *
     * \sa TilesetDocument::tileWangSetChanged
     */
    void tilesChanged(const QList<Tile*> &tiles);

private:
    /**
     * Should be called when anything changes about the given \a tile that
     * affects its display in any views on this model.
     *
     * \sa TilesetDocument::tileAnimationChanged
     * \sa TilesetDocument::tileImageSourceChanged
     */
    void tileChanged(Tile *tile);

    void refreshTileIds();

    TilesetDocument *mTilesetDocument;
    QList<int> mTileIds;
    int mColumnCountOverride = 0;
};

} // namespace Tiled
