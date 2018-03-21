/*
 * layermodel.cpp
 * Copyright 2008-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "changelayer.h"
#include "grouplayer.h"
#include "map.h"
#include "mapdocument.h"
#include "layer.h"
#include "renamelayer.h"
#include "reparentlayers.h"
#include "tilelayer.h"

#include <QApplication>
#include <QMimeData>
#include <QStyle>

using namespace Tiled;
using namespace Tiled::Internal;

LayerModel::LayerModel(QObject *parent):
    QAbstractItemModel(parent),
    mMapDocument(nullptr),
    mMap(nullptr),
    mTileLayerIcon(QLatin1String(":/images/16x16/layer-tile.png")),
    mObjectGroupIcon(QLatin1String(":/images/16x16/layer-object.png")),
    mImageLayerIcon(QLatin1String(":/images/16x16/layer-image.png"))
{
    mTileLayerIcon.addFile(QLatin1String(":images/32x32/layer-tile.png"));
    mObjectGroupIcon.addFile(QLatin1String(":images/32x32/layer-object.png"));
}

QModelIndex LayerModel::index(int row, int column, const QModelIndex &parent) const
{
    // Top-level layer index
    if (!parent.isValid()) {
        if (row < mMap->layerCount())
            return createIndex(row, column, nullptr);
        return QModelIndex();
    }

    // Child of a group layer index
    Layer *layer = toLayer(parent);
    Q_ASSERT(layer);
    if (GroupLayer *groupLayer = layer->asGroupLayer())
        if (row < groupLayer->layerCount())
            return createIndex(row, column, groupLayer);

    return QModelIndex();
}

QModelIndex LayerModel::parent(const QModelIndex &index) const
{
    if (auto groupLayer = static_cast<GroupLayer*>(index.internalPointer()))
        return LayerModel::index(groupLayer);
    return QModelIndex();
}

/**
 * Returns the number of rows.
 */
int LayerModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        Layer *layer = toLayer(parent);
        Q_ASSERT(layer);
        if (GroupLayer *groupLayer = layer->asGroupLayer())
            return groupLayer->layerCount();
        return 0;
    }

    return mMap ? mMap->layerCount() : 0;
}

int LayerModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 3;
}

/**
 * Returns the data stored under the given <i>role</i> for the item
 * referred to by the <i>index</i>.
 */
QVariant LayerModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0)
        return QVariant();

    Layer *layer = toLayer(index);

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        if (index.column() == 0)
            return layer->name();
        break;
    case Qt::DecorationRole:
        if (index.column() == 0) {
            switch (layer->layerType()) {
            case Layer::TileLayerType:
                return mTileLayerIcon;
            case Layer::ObjectGroupType:
                return mObjectGroupIcon;
            case Layer::ImageLayerType:
                return mImageLayerIcon;
            case Layer::GroupLayerType:
                return QApplication::style()->standardIcon(QStyle::SP_DirIcon);
            }
        }
        break;
    case Qt::CheckStateRole:
        if (index.column() == 1)
            return layer->isVisible() ? Qt::Checked : Qt::Unchecked;
        if (index.column() == 2)
            return layer->isLocked() ? Qt::Checked : Qt::Unchecked;
        break;
    case OpacityRole:
        return layer->opacity();
    default:
        return QVariant();
    }

    return QVariant();
}

/**
 * Allows for changing the name, visibility and opacity of a layer.
 */
bool LayerModel::setData(const QModelIndex &index, const QVariant &value,
                         int role)
{
    if (!index.isValid())
        return false;

    Layer *layer = toLayer(index);

    if (role == Qt::CheckStateRole) {
        if (index.column() == 1) {
            Qt::CheckState c = static_cast<Qt::CheckState>(value.toInt());
            const bool visible = (c == Qt::Checked);
            if (visible != layer->isVisible()) {
                QUndoCommand *command = new SetLayerVisible(mMapDocument,
                                                            layer,
                                                            visible);
                mMapDocument->undoStack()->push(command);
            }
        }
        if (index.column() == 2) {
            Qt::CheckState c = static_cast<Qt::CheckState>(value.toInt());
            const bool locked = (c == Qt::Checked);
            if (locked != layer->isLocked()) {
                QUndoCommand *command = new SetLayerLocked(mMapDocument,
                                                           layer,
                                                           locked);
                mMapDocument->undoStack()->push(command);
            }
        }
        return true;
    } else if (role == OpacityRole) {
        bool ok;
        const qreal opacity = value.toDouble(&ok);
        if (ok) {
            if (layer->opacity() != opacity) {
                QUndoCommand *command = new SetLayerOpacity(mMapDocument,
                                                            layer,
                                                            opacity);
                mMapDocument->undoStack()->push(command);
            }
            return true;
        }
    } else if (role == Qt::EditRole) {
        const QString newName = value.toString();
        if (layer->name() != newName) {
            RenameLayer *rename = new RenameLayer(mMapDocument, layer,
                                                  newName);
            mMapDocument->undoStack()->push(rename);
        }
        return true;
    }

    return false;
}

/**
 * Makes sure the items are checkable and names editable.
 */
Qt::ItemFlags LayerModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags rc = QAbstractItemModel::flags(index);

    if (index.column() == 1 || index.column() == 2)
        rc |= Qt::ItemIsUserCheckable;

    if (index.column() == 0)
        rc |= Qt::ItemIsEditable;

    Layer *layer = toLayer(index);

    if (layer)                              // can drag any layer
        rc |= Qt::ItemIsDragEnabled;

    if (!layer || layer->isGroupLayer())    // can drop on map or group layer
        rc |= Qt::ItemIsDropEnabled;

    return rc;
}

/**
 * Returns the headers for the table.
 */
QVariant LayerModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return tr("Layer");
        case 1: return tr("Visible");
        case 2: return tr("Locked");
        }
    }
    return QVariant();
}

QStringList LayerModel::mimeTypes() const
{
    return QStringList {
        QLatin1String(LAYERS_MIMETYPE)
    };
}

QMimeData *LayerModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.isEmpty())
        return nullptr;

    QMimeData *mimeData = new QMimeData;
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex &index : indexes)
        if (Layer *layer = toLayer(index))
            stream << globalIndex(layer);

    mimeData->setData(QLatin1String(LAYERS_MIMETYPE), encodedData);
    return mimeData;
}

Qt::DropActions LayerModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

bool LayerModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                              int row, int column, const QModelIndex &parent)
{
    if (!data || action != Qt::MoveAction)
        return false;
    if (!data->hasFormat(QLatin1String(LAYERS_MIMETYPE)))
        return false;

    Layer *parentLayer = toLayer(parent);
    if (parentLayer && !parentLayer->isGroupLayer())
        return false;

    GroupLayer *groupLayer = static_cast<GroupLayer*>(parentLayer);

    QByteArray encodedData = data->data(QLatin1String(LAYERS_MIMETYPE));
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QList<Layer*> layers;

    while (!stream.atEnd()) {
        int globalIndex;
        stream >> globalIndex;
        if (Layer *layer = layerAtGlobalIndex(mMap, globalIndex))
            layers.append(layer);
    }

    if (layers.isEmpty())
        return false;

    if (row > rowCount(parent))
        row = rowCount(parent);
    if (row == -1)
        row = groupLayer ? groupLayer->layerCount() : 0;
    if (column == -1)
        column = 0;

    // NOTE: QAbstractItemView::dropEvent already makes sure that we're not
    // dropping onto ourselves (like putting a group layer into itself).

    auto command = new ReparentLayers(mMapDocument, layers, groupLayer, row);
    command->setText(tr("Drag Layer(s)", nullptr, layers.size()));

    mMapDocument->undoStack()->push(command);

    return true;
}

QModelIndex LayerModel::index(Layer *layer, int column) const
{
    if (!layer)
        return QModelIndex();

    Q_ASSERT(layer->map() == mMap);

    if (auto parentLayer = layer->parentLayer()) {
        int row = parentLayer->layers().indexOf(layer);
        Q_ASSERT(row != -1);
        return createIndex(row, column, parentLayer);
    }

    int row = mMap->layers().indexOf(layer);
    Q_ASSERT(row != -1);
    return createIndex(row, column, nullptr);
}

Layer *LayerModel::toLayer(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    if (auto groupLayer = static_cast<GroupLayer*>(index.internalPointer()))
        return groupLayer->layerAt(index.row());

    return mMap->layerAt(index.row());
}

/**
 * Sets the map document associated with this model.
 */
void LayerModel::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    beginResetModel();
    mMapDocument = mapDocument;
    mMap = mMapDocument->map();
    endResetModel();
}

/**
 * Adds a layer to this model's map, inserting it at as a child of
 * \a parentLayer at the given \a index.
 */
void LayerModel::insertLayer(GroupLayer *parentLayer, int index, Layer *layer)
{
    QModelIndex parent = LayerModel::index(parentLayer);
    beginInsertRows(parent, index, index);
    if (parentLayer)
        parentLayer->insertLayer(index, layer);
    else
        mMap->insertLayer(index, layer);
    endInsertRows();
    emit layerAdded(layer);
}

/**
 * Removes the layer at the given \a index from this model's map and
 * returns it. The caller becomes responsible for the lifetime of this
 * layer.
 */
Layer *LayerModel::takeLayerAt(GroupLayer *parentLayer, int index)
{
    emit layerAboutToBeRemoved(parentLayer, index);
    QModelIndex parent = LayerModel::index(parentLayer);
    beginRemoveRows(parent, index, index);
    Layer *layer;
    if (parentLayer)
        layer = parentLayer->takeLayerAt(index);
    else
        layer = mMap->takeLayerAt(index);
    endRemoveRows();
    emit layerRemoved(layer);
    return layer;
}

/**
 * Replaces the given \a layer with the \a replacement layer.
 *
 * The map will take ownership over the replacement layer, whereas the caller
 * becomes responsible for the replaced layer.
 */
void LayerModel::replaceLayer(Layer *layer, Layer *replacement)
{
    Q_ASSERT(layer->map() == mMapDocument->map());
    Q_ASSERT(!replacement->map());

    auto currentLayer = mMapDocument->currentLayer();

    auto parentLayer = layer->parentLayer();
    auto index = layer->siblingIndex();

    takeLayerAt(parentLayer, index);
    insertLayer(parentLayer, index, replacement);

    if (layer == currentLayer)
        mMapDocument->setCurrentLayer(replacement);
}

/**
 * Sets whether the layer at the given index is visible.
 */
void LayerModel::setLayerVisible(Layer *layer, bool visible)
{
    if (layer->isVisible() == visible)
        return;

    layer->setVisible(visible);

    const QModelIndex modelIndex = index(layer, 1);
    emit dataChanged(modelIndex, modelIndex);
    emit layerChanged(layer);
}

void LayerModel::setLayerLocked(Layer *layer, bool locked)
{
    if (layer->isLocked() == locked)
        return;

    layer->setLocked(locked);

    const QModelIndex modelIndex = index(layer, 2);
    emit dataChanged(modelIndex, modelIndex);
    emit layerChanged(layer);
}

/**
 * Sets the opacity of the layer at the given index.
 */
void LayerModel::setLayerOpacity(Layer *layer, qreal opacity)
{
    if (layer->opacity() == opacity)
        return;

    layer->setOpacity(opacity);
    emit layerChanged(layer);
}

/**
 * Sets the offset of the layer at the given index.
 */
void LayerModel::setLayerOffset(Layer *layer, const QPointF &offset)
{
    if (layer->offset() == offset)
        return;

    layer->setOffset(offset);
    emit layerChanged(layer);
}

/**
 * Renames the layer at the given index.
 */
void LayerModel::renameLayer(Layer *layer, const QString &name)
{
    if (layer->name() == name)
        return;

    layer->setName(name);

    const QModelIndex modelIndex = index(layer);
    emit dataChanged(modelIndex, modelIndex);
    emit layerChanged(layer);
}

/**
 * Collects sibling layers, including siblings of all parents.
 */
static QList<Layer *> collectAllSiblings(Layer *layer)
{
    QList<Layer *> collected;

    while (layer) {
        const auto& siblings = layer->siblings();
        for (Layer *sibling : siblings) {
            if (sibling != layer)
                collected.append(sibling);
        }
        layer = layer->parentLayer();
    }

    return collected;
}

/**
  * Show or hide all other layers except the given \a layer.
  * If any other layer is visible then all layers will be hidden, otherwise
  * the layers will be shown.
  */
void LayerModel::toggleOtherLayers(Layer *layer)
{
    const auto& otherLayers = collectAllSiblings(layer);
    if (otherLayers.isEmpty())
        return;

    bool visibility = true;
    for (Layer *l : otherLayers) {
        if (l->isVisible()) {
            visibility = false;
            break;
        }
    }

    QUndoStack *undoStack = mMapDocument->undoStack();
    if (visibility)
        undoStack->beginMacro(tr("Show Other Layers"));
    else
        undoStack->beginMacro(tr("Hide Other Layers"));

    for (Layer *l : otherLayers) {
        if (visibility != l->isVisible())
            undoStack->push(new SetLayerVisible(mMapDocument, l, visibility));
    }

    undoStack->endMacro();
}
