/*
 * layermodel.h
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#ifndef LAYERMODEL_H
#define LAYERMODEL_H

#include <QAbstractListModel>
#include <QIcon>

namespace Tiled {

class Layer;
class Map;

namespace Internal {

class MapDocument;

/**
 * A model wrapping the layers of a map. Used to display the layers in a view.
 * The model also allows modification of the layer stack while keeping the
 * layer views up to date.
 */
class LayerModel : public QAbstractListModel
{
    Q_OBJECT

public:
    /**
     * The OpacityRole allows querying and changing the layer opacity.
     */
    enum UserRoles {
        OpacityRole = Qt::UserRole
    };

    /**
     * Constructor.
     */
    LayerModel(QObject *parent = 0);

    /**
     * Returns the number of rows.
     */
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    /**
     * Returns the data stored under the given <i>role</i> for the item
     * referred to by the <i>index</i>.
     */
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const;

    /**
     * Allows for changing the name, visibility and opacity of a layer.
     */
    bool setData(const QModelIndex &index, const QVariant &value, int role);

    /**
     * Makes sure the items are checkable and names editable.
     */
    Qt::ItemFlags flags(const QModelIndex &index) const;

    /**
     * Returns the headers for the table.
     */
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;

    /**
     * Returns the layer index associated with a given model index.
     * \sa layerIndexToRow
     */
    int toLayerIndex(const QModelIndex &index) const;

    /**
     * Returns the row associated with the given layer index.
     * \sa toLayerIndex
     */
    int layerIndexToRow(int layerIndex) const;

    /**
     * Returns the map document associated with this model.
     */
    MapDocument *mapDocument() const { return mMapDocument; }

    /**
     * Sets the map document associated with this model.
     */
    void setMapDocument(MapDocument *mapDocument);

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

    /**
     * Sets whether the layer at the given index is visible.
     */
    void setLayerVisible(int layerIndex, bool visible);

    /**
     * Sets the opacity of the layer at the given index.
     */
    void setLayerOpacity(int layerIndex, float opacity);

    /**
     * Renames the layer at the given index.
     */
    void renameLayer(int index, const QString &name);

    /**
      * Show or hide all other layers except the layer at the given index.
      * If any other layer is visible then all layers will be hidden, otherwise
      * the layers will be shown.
      */
    void toggleOtherLayers(int layerIndex);

signals:
    void layerAdded(int index);
    void layerAboutToBeRemoved(int index);
    void layerRemoved(int index);
    void layerAboutToBeRenamed(int index);
    void layerRenamed(int index);
    void layerChanged(int index);

private:
    MapDocument *mMapDocument;
    Map *mMap;

    QIcon mTileLayerIcon;
    QIcon mObjectGroupIcon;
    QIcon mImageLayerIcon;
};

} // namespace Internal
} // namespace Tiled

#endif // LAYERMODEL_H
