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
    if (role == Qt::DisplayRole) {
        const int layerIndex = toLayerIndex(index);
        if (layerIndex >= 0)
            return mMap->layers().at(layerIndex)->name();
    }
    return QVariant();
}

QVariant LayerTableModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const
{
    if (role == Qt::DisplayRole
            && section == 0
            && orientation == Qt::Horizontal) {
        return tr("Name");
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
    if (mMap != map) {
        mMap = map;
        reset();
    }
}
