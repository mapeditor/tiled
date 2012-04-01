/*
 * layermodel.cpp
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

#include "layermodel.h"

#include "map.h"
#include "mapdocument.h"
#include "layer.h"
#include "renamelayer.h"
#include "tilelayer.h"


using namespace Tiled;
using namespace Tiled::Internal;

LayerModel::LayerModel(QObject *parent):
    QAbstractListModel(parent),
    mMapDocument(0),
    mMap(0),
    mTileLayerIcon(QLatin1String(":/images/16x16/layer-tile.png")),
    mObjectGroupIcon(QLatin1String(":/images/16x16/layer-object.png")),
    mImageLayerIcon(QLatin1String(":/images/16x16/layer-image.png"))
{
}

int LayerModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : (mMap ? mMap->layerCount() : 0);
}

QVariant LayerModel::data(const QModelIndex &index, int role) const
{
    const int layerIndex = toLayerIndex(index);
    if (layerIndex < 0)
        return QVariant();

    const Layer *layer = mMap->layerAt(layerIndex);

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return layer->name();
    case Qt::DecorationRole:
        if (layer->isTileLayer())
            return mTileLayerIcon;
        else if (layer->isObjectGroup())
            return mObjectGroupIcon;
        else if (layer->isImageLayer())
            return mImageLayerIcon;
        else
            Q_ASSERT(false);
    case Qt::CheckStateRole:
        return layer->isVisible() ? Qt::Checked : Qt::Unchecked;
    case OpacityRole:
        return layer->opacity();
    default:
        return QVariant();
    }
}

bool LayerModel::setData(const QModelIndex &index, const QVariant &value,
                         int role)
{
    const int layerIndex = toLayerIndex(index);
    if (layerIndex < 0)
        return false;

    Layer *layer = mMap->layerAt(layerIndex);

    if (role == Qt::CheckStateRole) {
        Qt::CheckState c = static_cast<Qt::CheckState>(value.toInt());
        layer->setVisible(c == Qt::Checked);
        emit dataChanged(index, index);
        emit layerChanged(layerIndex);
        return true;
    } else if (role == OpacityRole) {
        bool ok;
        const qreal opacity = value.toDouble(&ok);
        if (ok) {
            if (layer->opacity() != opacity) {
                layer->setOpacity(opacity);
                emit layerChanged(layerIndex);
            }
        }
    } else if (role == Qt::EditRole) {
        const QString newName = value.toString();
        if (layer->name() != newName) {
            RenameLayer *rename = new RenameLayer(mMapDocument, layerIndex,
                                                  value.toString());
            mMapDocument->undoStack()->push(rename);
        }
        return true;
    }

    return false;
}

Qt::ItemFlags LayerModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags rc = QAbstractListModel::flags(index);
    if (index.column() == 0)
        rc |= Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
    return rc;
}

QVariant LayerModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return tr("Layer");
        }
    }
    return QVariant();
}

int LayerModel::toLayerIndex(const QModelIndex &index) const
{
    if (index.isValid()) {
        return mMap->layerCount() - index.row() - 1;
    } else {
        return -1;
    }
}

int LayerModel::layerIndexToRow(int layerIndex) const
{
    return mMap->layerCount() - layerIndex - 1;
}

void LayerModel::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    mMapDocument = mapDocument;
    mMap = mMapDocument->map();
    reset();
}

void LayerModel::insertLayer(int index, Layer *layer)
{
    const int row = layerIndexToRow(index) + 1;
    beginInsertRows(QModelIndex(), row, row);
    mMap->insertLayer(index, layer);
    endInsertRows();
    emit layerAdded(index);
}

Layer *LayerModel::takeLayerAt(int index)
{
    emit layerAboutToBeRemoved(index);
    const int row = layerIndexToRow(index);
    beginRemoveRows(QModelIndex(), row, row);
    Layer *layer = mMap->takeLayerAt(index);
    endRemoveRows();
    emit layerRemoved(index);
    return layer;
}

void LayerModel::renameLayer(int layerIndex, const QString &name)
{
    emit layerAboutToBeRenamed(layerIndex);
    const QModelIndex modelIndex = index(layerIndexToRow(layerIndex), 0);
    Layer *layer = mMap->layerAt(layerIndex);
    layer->setName(name);
    emit layerRenamed(layerIndex);
    emit dataChanged(modelIndex, modelIndex);
    emit layerChanged(layerIndex);
}

void LayerModel::toggleOtherLayers(int layerIndex)
{
    bool visibility = true;
    for (int i = 0; i < mMap->layerCount(); i++) {
        if (i == layerIndex)
            continue;

        Layer *layer = mMap->layerAt(i);
        if (layer->isVisible()) {
            visibility = false;
            break;
        }
    }

    for (int i = 0; i < mMap->layerCount(); i++) {
        if (i == layerIndex)
            continue;

        const QModelIndex modelIndex = index(layerIndexToRow(i), 0);
        Layer *layer = mMap->layerAt(i);
        layer->setVisible(visibility);
        emit dataChanged(modelIndex, modelIndex);
        emit layerChanged(i);
    }
}
