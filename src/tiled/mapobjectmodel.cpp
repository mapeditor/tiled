/*
 * mapobjectmodel.cpp
 * Copyright 2012, Tim Baker <treectrl@hotmail.com>
 * Copyright 2012-2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "mapobjectmodel.h"

#include "changeevents.h"
#include "changelayer.h"
#include "changemapobject.h"
#include "changeproperties.h"
#include "grouplayer.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "objectgroup.h"

#include <QApplication>
#include <QPalette>
#include <QStyle>

using namespace Tiled;

ObjectIconManager::ObjectIconManager()
   : mRectangleIcon(QLatin1String(":images/24/object-rectangle.png"))
   , mImageIcon(QLatin1String(":images/24/object-image.png"))
   , mPolygonIcon(QLatin1String(":images/24/object-polygon.png"))
   , mPolylineIcon(QLatin1String(":images/24/object-polyline.png"))
   , mEllipseIcon(QLatin1String(":images/24/object-ellipse.png"))
   , mTextIcon(QLatin1String(":images/24/object-text.png"))
   , mPointIcon(QLatin1String(":images/24/object-point.png"))
{}

const ObjectIconManager &ObjectIconManager::instance()
{
    static ObjectIconManager objectIconManager;
    return objectIconManager;
}

const QIcon &ObjectIconManager::iconForObject(const MapObject &object) const
{
    switch (object.shape()) {
    case MapObject::Rectangle:
        return object.isTileObject() ? mImageIcon : mRectangleIcon;
    case MapObject::Polygon:
        return mPolygonIcon;
    case MapObject::Polyline:
        return mPolylineIcon;
    case MapObject::Ellipse:
        return mEllipseIcon;
    case MapObject::Text:
        return mTextIcon;
    case MapObject::Point:
        return mPointIcon;
    }
    return mImageIcon;
}


MapObjectModel::MapObjectModel(QObject *parent)
    : QAbstractItemModel(parent)
    , mMapDocument(nullptr)
    , mMap(nullptr)
    , mObjectGroupIcon(QLatin1String(":/images/16/layer-object.png"))
{
    mObjectGroupIcon.addFile(QLatin1String(":images/32/layer-object.png"));
}

QModelIndex MapObjectModel::index(int row, int column,
                                  const QModelIndex &parent) const
{
    if (ObjectGroup *objectGroup = toObjectGroup(parent)) {
        if (row < objectGroup->objectCount())
            return createIndex(row, column, objectGroup->objectAt(row));
        return QModelIndex();
    }

    GroupLayer *parentLayer = toGroupLayer(parent); // may be nullptr
    const QList<Layer *> &layers = filteredChildLayers(parentLayer);

    if (row < layers.size())
        return createIndex(row, column, layers.at(row));

    return QModelIndex();
}

QModelIndex MapObjectModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    Object *object = static_cast<Object*>(index.internalPointer());
    switch (object->typeId()) {
    case Object::LayerType:
        if (Layer *layer = static_cast<Layer*>(object)->parentLayer())
            return this->index(layer);
        break;
    case Object::MapObjectType:
        return this->index(static_cast<MapObject*>(object)->objectGroup());
    default:
        break;
    }

    return QModelIndex();
}

int MapObjectModel::rowCount(const QModelIndex &parent) const
{
    if (!mMapDocument)
        return 0;
    if (!parent.isValid())
        return filteredChildLayers(nullptr).size();

    Object *object = static_cast<Object*>(parent.internalPointer());
    if (object->typeId() == Object::LayerType) {
        Layer *layer = static_cast<Layer*>(object);
        switch (layer->layerType()) {
        case Layer::GroupLayerType:
            return filteredChildLayers(static_cast<GroupLayer*>(layer)).size();
        case Layer::ObjectGroupType:
            return static_cast<ObjectGroup*>(layer)->objectCount();
        default:
            break;
        }
    }
    return 0;
}

int MapObjectModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return ColumnCount;
}

QVariant MapObjectModel::data(const QModelIndex &index, int role) const
{
    if (MapObject *mapObject = toMapObject(index)) {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            switch (index.column()) {
            case Name: {
                QString name = mapObject->name();

                // Display the tile image file name as a fallback
                if (name.isEmpty())
                    if (Tile *tile = mapObject->cell().tile())
                        if (!tile->imageSource().isEmpty())
                            name = QFileInfo(tile->imageSource().fileName()).completeBaseName();

                return name;
            }
            case Class:
                return mapObject->effectiveClassName();
            case Id:
                return mapObject->id();
            case Position:
                return QLatin1Char('(')
                        + QString::number(mapObject->x())
                        + QLatin1String(", ")
                        + QString::number(mapObject->y())
                        + QLatin1Char(')');
            }
            break;
        case Qt::DecorationRole:
            if (index.column() == Name)
                return ObjectIconManager::instance().iconForObject(*mapObject);
            break;
        case Qt::ForegroundRole: {
            bool disabled = false;

            switch (index.column()) {
            case Name:
                disabled = mapObject->name().isEmpty();
                break;
            case Class:
                disabled = mapObject->className().isEmpty();
                break;
            }

            if (disabled) {
                const QPalette palette = QApplication::palette();
                return palette.brush(QPalette::Disabled, QPalette::WindowText);
            }

            return QVariant();
        }
        case Qt::CheckStateRole:
            if (index.column() > 0)
                return QVariant();
            return mapObject->isVisible() ? Qt::Checked : Qt::Unchecked;
        case OpacityRole:
            return qreal(1);
        default:
            return QVariant();
        }
    }

    if (Layer *layer = toLayer(index)) {
        switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return index.column() ? QVariant() : layer->name();
        case Qt::DecorationRole:
            if (index.column() == 0) {
                if (layer->isObjectGroup())
                    return mObjectGroupIcon;
                else
                    return QApplication::style()->standardIcon(QStyle::SP_DirIcon);
            }
            return QVariant();
        case Qt::CheckStateRole:
            if (index.column() > 0)
                return QVariant();
            return layer->isVisible() ? Qt::Checked : Qt::Unchecked;
        case OpacityRole:
            return layer->opacity();
        default:
            return QVariant();
        }
    }

    return QVariant();
}

bool MapObjectModel::setData(const QModelIndex &index, const QVariant &value,
                             int role)
{
    if (MapObject *mapObject = toMapObject(index)) {
        switch (role) {
        case Qt::CheckStateRole: {
            Qt::CheckState c = static_cast<Qt::CheckState>(value.toInt());
            const bool visible = (c == Qt::Checked);
            if (visible != mapObject->isVisible()) {
                QUndoCommand *command = new ChangeMapObject(mMapDocument,
                                                            mapObject,
                                                            MapObject::VisibleProperty,
                                                            visible);
                mMapDocument->undoStack()->push(command);
            }
            return true;
        }
        case Qt::EditRole: {
            const QString s = value.toString();
            if (index.column() == 0 && s != mapObject->name()) {
                QUndoStack *undo = mMapDocument->undoStack();
                undo->beginMacro(tr("Change Object Name"));
                undo->push(new ChangeMapObject(mMapDocument, mapObject,
                                               MapObject::NameProperty, s));
                undo->endMacro();
            }
            if (index.column() == 1 && s != mapObject->className()) {
                QUndoStack *undo = mMapDocument->undoStack();
                undo->push(new ChangeClassName(mMapDocument, { mapObject }, s));
            }
            return true;
        }
        }
        return false;
    }
    if (Layer *layer = toLayer(index)) {
        switch (role) {
        case Qt::CheckStateRole: {
            Qt::CheckState c = static_cast<Qt::CheckState>(value.toInt());
            const bool visible = (c == Qt::Checked);
            if (visible != layer->isVisible()) {
                QUndoCommand *command = new SetLayerVisible(mMapDocument,
                                                            { layer },
                                                            visible);
                mMapDocument->undoStack()->push(command);
            }
            return true;
        }
        case Qt::EditRole: {
            const QString newName = value.toString();
            if (layer->name() != newName) {
                SetLayerName *rename = new SetLayerName(mMapDocument,
                                                        { layer },
                                                        newName);
                mMapDocument->undoStack()->push(rename);
            }
            return true;
        }
        }
        return false;
    }
    return false;
}

Qt::ItemFlags MapObjectModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags rc = QAbstractItemModel::flags(index);
    if (index.column() == 0)
        rc |= Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
    else if (toMapObject(index)) {
        if (index.column() == Class)    // allow to edit only class column
            rc |= Qt::ItemIsEditable;
    }
    return rc;
}

QVariant MapObjectModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
        case Name: return tr("Name");
        case Class: return tr("Class");
        case Id: return tr("ID");
        case Position: return tr("Position");
        }
    }
    return QVariant();
}

QModelIndex MapObjectModel::index(Layer *layer) const
{
    Q_ASSERT(layer);
    Q_ASSERT(layer->isObjectGroup() || layer->isGroupLayer());
    Q_ASSERT(layer->map() == mMap);

    const int row = filteredChildLayers(layer->parentLayer()).indexOf(layer);
    return createIndex(row, 0, layer);
}

QModelIndex MapObjectModel::index(MapObject *mapObject, int column) const
{
    Q_ASSERT(mapObject);
    Q_ASSERT(mapObject->objectGroup());
    Q_ASSERT(mapObject->map() == mMap);

    const int row = mapObject->objectGroup()->objects().indexOf(mapObject);
    return createIndex(row, column, mapObject);
}

Layer *MapObjectModel::toLayer(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    Object *object = static_cast<Object*>(index.internalPointer());
    if (object->typeId() == Object::LayerType)
        return static_cast<Layer*>(object);

    return nullptr;
}

MapObject *MapObjectModel::toMapObject(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    Object *object = static_cast<Object*>(index.internalPointer());
    if (object->typeId() == Object::MapObjectType)
        return static_cast<MapObject*>(object);

    return nullptr;
}

ObjectGroup *MapObjectModel::toObjectGroup(const QModelIndex &index) const
{
    if (Layer *layer = toLayer(index))
        return layer->asObjectGroup();
    return nullptr;
}

GroupLayer *MapObjectModel::toGroupLayer(const QModelIndex &index) const
{
    if (Layer *layer = toLayer(index))
        return layer->asGroupLayer();
    return nullptr;
}

ObjectGroup *MapObjectModel::toObjectGroupContext(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    Object *object = static_cast<Object*>(index.internalPointer());
    if (object->typeId() == Object::LayerType) {
        if (auto objectGroup = static_cast<Layer*>(object)->asObjectGroup())
            return objectGroup;
    } else if (object->typeId() == Object::MapObjectType) {
        return static_cast<MapObject*>(object)->objectGroup();
    }

    return nullptr;
}

void MapObjectModel::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    if (mMapDocument)
        mMapDocument->disconnect(this);

    beginResetModel();
    mMapDocument = mapDocument;
    mMap = nullptr;

    mFilteredLayers.clear();

    if (mMapDocument) {
        mMap = mMapDocument->map();

        connect(mMapDocument, &MapDocument::layerAdded,
                this, &MapObjectModel::layerAdded);
        connect(mMapDocument, &MapDocument::layerAboutToBeRemoved,
                this, &MapObjectModel::layerAboutToBeRemoved);

        connect(mMapDocument, &Document::changed,
                this, &MapObjectModel::documentChanged);
    }

    endResetModel();
}

void MapObjectModel::layerAdded(Layer *layer)
{
    if (layer->isObjectGroup() || layer->isGroupLayer()) {
        auto &filtered = filteredChildLayers(layer->parentLayer());
        if (filtered.contains(layer))
            return;

        const auto &siblings = layer->siblings();

        Layer *prev = nullptr;
        for (int i = siblings.indexOf(layer) - 1; i >= 0; --i) {
            auto sibling = siblings.at(i);
            if (sibling->isObjectGroup() || sibling->isGroupLayer()) {
                prev = sibling;
                break;
            }
        }

        int row = prev ? filtered.indexOf(prev) + 1 : 0;

        QModelIndex parent;
        if (layer->parentLayer())
            parent = index(layer->parentLayer());

        beginInsertRows(parent, row, row);
        filtered.insert(row, layer);
        endInsertRows();
    }
}

void MapObjectModel::layerAboutToBeRemoved(GroupLayer *groupLayer, int index)
{
    const auto &layers = groupLayer ? groupLayer->layers() : mMap->layers();
    Layer *layer = layers.at(index);

    if (layer->isObjectGroup() || layer->isGroupLayer()) {
        auto &filtered = filteredChildLayers(groupLayer);
        const int row = filtered.indexOf(layer);

        QModelIndex parent = groupLayer ? this->index(groupLayer) : QModelIndex();

        beginRemoveRows(parent, row, row);
        filtered.removeAt(row);
        endRemoveRows();
    }
}

void MapObjectModel::classChanged(const QList<Object *> &objects)
{
    if (objects.isEmpty())
        return;

    const auto typeId = objects.first()->typeId();

    QList<MapObject *> affectedObjects;
    affectedObjects.reserve(objects.size());

    if (typeId == Object::MapObjectType) {
        for (Object *object : objects)
            affectedObjects.append(static_cast<MapObject*>(object));
    } else if (typeId == Object::TileType) {
        for (const Layer *layer : mMap->objectGroups()) {
            auto objectGroup = static_cast<const ObjectGroup*>(layer);
            for (MapObject *mapObject : objectGroup->objects()) {
                if (mapObject->className().isEmpty())
                    if (auto tile = mapObject->cell().tile())
                        if (objects.contains(tile))
                            affectedObjects.append(mapObject);
            }
        }
    }

    QVarLengthArray<Column, 3> columns;
    columns.append(MapObjectModel::Class);

    emitDataChanged(affectedObjects, columns);
}

QList<Layer *> &MapObjectModel::filteredChildLayers(GroupLayer *parentLayer) const
{
    if (!mFilteredLayers.contains(parentLayer)) {
        QList<Layer*> &filtered = mFilteredLayers[parentLayer];
        const auto &layers = parentLayer ? parentLayer->layers() : mMap->layers();
        for (Layer *layer : layers)
            if (layer->isObjectGroup() || layer->isGroupLayer())
                filtered.append(layer);
        return filtered;
    }

    return mFilteredLayers[parentLayer];
}

void MapObjectModel::moveObjects(ObjectGroup *og, int from, int to, int count)
{
    const QModelIndex parent = index(og);
    if (!beginMoveRows(parent, from, from + count - 1, parent, to)) {
        Q_ASSERT(false); // The code should never attempt this
        return;
    }

    og->moveObjects(from, to, count);
    endMoveRows();
}

void MapObjectModel::documentChanged(const ChangeEvent &change)
{
    // Notify views about certain property changes
    switch (change.type) {
    case ChangeEvent::ObjectsChanged: {
        auto &objectsChange = static_cast<const ObjectsChangeEvent&>(change);
        if (objectsChange.properties & ObjectsChangeEvent::ClassProperty)
            classChanged(objectsChange.objects);
        break;
    }
    case ChangeEvent::LayerChanged: {
        auto &layerChange = static_cast<const LayerChangeEvent&>(change);

        if (layerChange.properties & (LayerChangeEvent::NameProperty | LayerChangeEvent::VisibleProperty)) {
            auto layer = layerChange.layer;
            if (layer->isObjectGroup() || layer->isGroupLayer()) {
                QModelIndex index = this->index(layer);
                emit dataChanged(index, index);
            }
        }

        break;
    }
    case ChangeEvent::MapObjectsAboutToBeRemoved:
    case ChangeEvent::MapObjectsAdded:
    case ChangeEvent::MapObjectsRemoved:
        // handled individually instead
        break;
    case ChangeEvent::MapObjectAboutToBeAdded: {
        auto &e = static_cast<const MapObjectEvent&>(change);
        beginInsertRows(index(e.objectGroup), e.index, e.index);
        break;
    }
    case ChangeEvent::MapObjectAboutToBeRemoved: {
        auto &e = static_cast<const MapObjectEvent&>(change);
        beginRemoveRows(index(e.objectGroup), e.index, e.index);
        break;
    }
    case ChangeEvent::MapObjectAdded:
        endInsertRows();
        break;
    case ChangeEvent::MapObjectRemoved:
        endRemoveRows();
        break;
    case ChangeEvent::MapObjectsChanged: {
        const auto &mapObjectChange = static_cast<const MapObjectsChangeEvent&>(change);

        QVarLengthArray<Column, 3> columns;
        if (mapObjectChange.properties & (MapObject::NameProperty | MapObject::VisibleProperty))
            columns.append(MapObjectModel::Name);
        if (mapObjectChange.properties & MapObject::PositionProperty)
            columns.append(MapObjectModel::Position);

        emitDataChanged(mapObjectChange.mapObjects, columns);

        break;
    }
    default:
        break;
    }
}

void MapObjectModel::emitDataChanged(const QList<MapObject *> &objects,
                                     const QVarLengthArray<Column, 3> &columns,
                                     const QVector<int> &roles)
{
    if (columns.isEmpty())
        return;

    auto minMaxPair = std::minmax_element(columns.begin(), columns.end());
    for (auto object : objects) {
        emit dataChanged(index(object, *minMaxPair.first),
                         index(object, *minMaxPair.second),
                         roles);
    }
}

#include "moc_mapobjectmodel.cpp"
