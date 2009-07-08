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

#ifndef LAYERTABLEMODEL_H
#define LAYERTABLEMODEL_H

#include <QAbstractTableModel>

namespace Tiled {

class Layer;
class Map;

namespace Internal {

/**
 * A model wrapping the layers of a map. Used to display the layers in a view.
 * The model also allows modification of the layer stack while keeping the
 * layer views up to date.
 */
class LayerTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    LayerTableModel(QObject *parent = 0);

    /**
     * Returns the number of rows.
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
     * Allows for changing the visibility of a layer.
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    /**
     * Makes sure the items are checkable.
     */
    Qt::ItemFlags flags(const QModelIndex &index) const;

    /**
     * Returns the headers for the table.
     */
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    /**
     * Returns the layer index associated with a given model index.
     */
    int toLayerIndex(const QModelIndex &index) const;

    /**
     * Returns the row associated with the given layer index.
     */
    int layerIndexToRow(int layerIndex) const;

    /**
     * Returns the map associated with this model.
     */
    Map *map() const { return mMap; }

    /**
     * Sets the map associated with this model.
     */
    void setMap(Map *map);

    /**
     * Adds a layer to this model's map, inserting it at the given index.
     */
    void insertLayer(int index, Layer *layer);

    /**
     * Removes the layer at the given index from this model's map and
     * returns it. The caller becomes responsible for the lifetime of this
     * layer.
     */
    Layer *takeLayerAt(int index);

signals:
    void layerAdded(int index);
    void layerRemoved(int index);
    void layerChanged(int index);

private:
    Map *mMap;
};

} // namespace Internal
} // namespace Tiled

#endif // LAYERTABLEMODEL_H
