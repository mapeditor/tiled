/*
 * mapobjectmodel.h
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

#pragma once

#include "mapobject.h"

#include <QAbstractItemModel>
#include <QIcon>

namespace Tiled {

class GroupLayer;
class Layer;
class MapObject;
class Map;
class ObjectGroup;

class ChangeEvent;
class MapDocument;

/**
 * Provides a tree view on the objects present on a map. Also has member
 * functions to modify objects that emit the appropriate signals to allow
 * the UI to update.
 */
class MapObjectModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum UserRoles {
        OpacityRole = Qt::UserRole
    };

    enum Column {
        Name,
        Type,
        Id,
        Position,
        LastColumn = Position,
        ColumnCount
    };

    MapObjectModel(QObject *parent = nullptr);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QModelIndex index(Layer *layer) const;
    QModelIndex index(MapObject *mapObject, int column = 0) const;

    Layer *toLayer(const QModelIndex &index) const;
    MapObject *toMapObject(const QModelIndex &index) const;
    ObjectGroup *toObjectGroup(const QModelIndex &index) const;
    GroupLayer *toGroupLayer(const QModelIndex &index) const;
    ObjectGroup *toObjectGroupContext(const QModelIndex &index) const;

    void setMapDocument(MapDocument *mapDocument);
    MapDocument *mapDocument() const { return mMapDocument; }

    void moveObjects(ObjectGroup *og, int from, int to, int count);

private:
    void layerAdded(Layer *layer);
    void layerAboutToBeRemoved(GroupLayer *groupLayer, int index);
    void tileTypeChanged(Tile *tile);
    void documentChanged(const ChangeEvent &change);

    void emitDataChanged(const QList<MapObject *> &objects,
                         const QVarLengthArray<Column, 3> &columns,
                         const QVector<int> &roles = QVector<int>());

    MapDocument *mMapDocument;
    Map *mMap;

    // cache
    mutable QMap<GroupLayer*, QList<Layer*>> mFilteredLayers;
    QList<Layer *> &filteredChildLayers(GroupLayer *parentLayer) const;

    QIcon mObjectGroupIcon;
    QIcon mRectangleIcon;
    QIcon mImageIcon;
    QIcon mPolygonIcon;
    QIcon mPolylineIcon;
    QIcon mEllipseIcon;
    QIcon mTextIcon;
    QIcon mPointIcon;
};

} // namespace Tiled
