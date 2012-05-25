/*
 * mapobjectmodel.h
 * Copyright 2012, Tim Baker <treectrl@hotmail.com>
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

#ifndef MAPOBJECTMODEL_H
#define MAPOBJECTMODEL_H

#include <QAbstractItemModel>
#include <QIcon>

namespace Tiled {

class MapObject;
class Map;
class ObjectGroup;

namespace Internal {

class MapDocument;

class MapObjectModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum UserRoles {
        OpacityRole = Qt::UserRole
    };

    struct ObjectOrGroup
    {
        ObjectOrGroup(ObjectGroup *g)
            : mGroup(g)
            , mObject(0)
        {
        }
        ObjectOrGroup(MapObject *o)
            : mGroup(0)
            , mObject(o)
        {
        }
        ObjectGroup *mGroup;
        MapObject *mObject;
    };

    MapObjectModel(QObject *parent = 0);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    bool setData(const QModelIndex &index, const QVariant &value, int role);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    Qt::ItemFlags flags(const QModelIndex &index) const;

    QModelIndex index(ObjectGroup *og) const;
    QModelIndex index(MapObject *o, int column = 0) const;

    ObjectGroup *toObjectGroup(const QModelIndex &index) const;
    MapObject *toMapObject(const QModelIndex &index) const;
    ObjectGroup *toLayer(const QModelIndex &index) const;

    int toRow(const ObjectGroup *objectGroup) const;
    int toRow(const MapObject *mapObject) const;

    void setMapDocument(MapDocument *mapDocument);
    MapDocument *mapDocument() const { return mMapDocument; }

    void insertObject(ObjectGroup *og, int index, MapObject *o);
    int removeObject(ObjectGroup *og, MapObject *o);
    void emitObjectsChanged(const QList<MapObject *> &objects);

    void setObjectName(MapObject *o, const QString &name);
    void setObjectType(MapObject *o, const QString &type);
    void setObjectPolygon(MapObject *o, const QPolygonF &polygon);
    void setObjectPosition(MapObject *o, const QPointF &pos);
    void setObjectSize(MapObject *o, const QSizeF &size);

signals:
    void objectsAdded(const QList<MapObject *> &objects);
    void objectsChanged(const QList<MapObject *> &objects);
    void objectsAboutToBeRemoved(const QList<MapObject *> &objects);
    void objectsRemoved(const QList<MapObject *> &objects);

private slots:
    void layerAdded(int index);
    void layerChanged(int index);
    void layerAboutToBeRemoved(int index);

private:
    MapDocument *mMapDocument;
    Map *mMap;
    QList<ObjectGroup*> mObjectGroups;
    QMap<MapObject*, ObjectOrGroup*> mObjects;
    QMap<ObjectGroup*, ObjectOrGroup*> mGroups;

    QIcon mObjectGroupIcon;
};

} // namespace Internal
} // namespace Tiled

#endif // MAPOBJECTMODEL_H
