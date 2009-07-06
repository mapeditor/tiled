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

#include "layertablemodel.h"
#include "map.h"
#include "layer.h"

using namespace Tiled;
using namespace Tiled::Internal;

LayerTableModel::LayerTableModel(QObject *parent):
    QAbstractTableModel(parent),
    mMap(0)
{
}

int LayerTableModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : (mMap ? mMap->layers().size() : 0);
}

int LayerTableModel::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : 1;
}

QVariant LayerTableModel::data(const QModelIndex &index, int role) const
{
    const int layerIndex = toLayerIndex(index);
    if (layerIndex < 0)
        return QVariant();

    const Layer *layer = mMap->layers().at(layerIndex);

    switch (role) {
    case Qt::DisplayRole:
        return layer->name();
    case Qt::CheckStateRole:
        return layer->isVisible() ? Qt::Checked : Qt::Unchecked;
    default:
        return QVariant();
    }
}

bool LayerTableModel::setData(const QModelIndex &index, const QVariant &value,
                              int role)
{
    const int layerIndex = toLayerIndex(index);
    if (role != Qt::CheckStateRole || layerIndex < 0)
        return false;

    Layer *layer = mMap->layers().at(layerIndex);
    Qt::CheckState c = static_cast<Qt::CheckState>(value.toInt());
    layer->setVisible(c == Qt::Checked);
    emit dataChanged(index, index);
    emit layerChanged(layerIndex);
    return true;
}

Qt::ItemFlags LayerTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags rc = QAbstractTableModel::flags(index);
    if (index.column() == 0)
        rc |= Qt::ItemIsUserCheckable;
    return rc;
}

QVariant LayerTableModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return tr("Layer");
        }
    }
    return QVariant();
}

int LayerTableModel::toLayerIndex(const QModelIndex &index) const
{
    if (index.isValid()) {
        return mMap->layers().size() - index.row() - 1;
    } else {
        return -1;
    }
}

void LayerTableModel::setMap(Map *map)
{
    if (mMap == map)
        return;
    mMap = map;
    reset();
}

void LayerTableModel::insertLayer(int index, Layer *layer)
{
    beginInsertRows(QModelIndex(), index, index);
    mMap->insertLayer(index, layer);
    endInsertRows();
    emit layerAdded(index);
}

Layer *LayerTableModel::takeLayerAt(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    Layer *layer = mMap->takeLayerAt(index);
    endRemoveRows();
    emit layerRemoved(index);
    return layer;
}
