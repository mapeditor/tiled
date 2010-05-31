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

#ifndef TILESETMODEL_H
#define TILESETMODEL_H

#include <QAbstractListModel>

namespace Tiled {

class Tile;
class Tileset;

namespace Internal {

/**
 * A model wrapping a tileset of a map. Used to display the tiles.
 */
class TilesetModel : public QAbstractListModel
{
public:
    /**
     * Constructor.
     *
     * @param tileset the initial tileset to display
     */
    TilesetModel(Tileset *tileset, QObject *parent = 0);

    /**
     * Returns the number of rows. This is equal to the number of tiles.
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    /**
     * Returns the number of columns.
     */
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    /**
     * Returns the data stored under the given <i>role</i> for the item
     * referred to by the <i>index</i>.
     */
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const;

    /**
     * Returns a small size hint, to prevent the headers from affecting the
     * minimum width and height of the sections.
     */
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    /**
     * Returns the tile at the given index.
     */
    Tile *tileAt(const QModelIndex &index) const;

    /**
     * Returns the tileset associated with this model.
     */
    Tileset *tileset() const { return mTileset; }

    /**
     * Sets the tileset associated with this model.
     */
    void setTileset(Tileset *tileset);

    /**
     * Performs a reset() on the model
     */
    void tilesetChanged() { reset(); }

private:
    Tileset *mTileset;
};

} // namespace Internal
} // namespace Tiled

#endif // TILESETMODEL_H
