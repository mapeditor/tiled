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

#ifndef TILESETMODEL_H
#define TILESETMODEL_H

#include <QAbstractListModel>

namespace Tiled {

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
     * Returns the tileset associated with this model.
     */
    Tileset *tileset() const { return mTileset; }

    /**
     * Sets the tileset associated with this model.
     */
    void setTileset(Tileset *tileset);

private:
    Tileset *mTileset;
};

} // namespace Internal
} // namespace Tiled

#endif // TILESETMODEL_H
