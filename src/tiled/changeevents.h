/*
 * changeevents.h
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "imagelayer.h"
#include "map.h"
#include "mapobject.h"
#include "tileset.h"
#include "wangset.h"

#include <QList>

namespace Tiled {

class Layer;
class WangSet;

class ChangeEvent
{
public:
    enum Type {
        ObjectsChanged,
        MapChanged,
        LayerChanged,
        TileLayerChanged,
        ImageLayerChanged,
        MapObjectAboutToBeAdded,
        MapObjectAboutToBeRemoved,
        MapObjectAdded,
        MapObjectRemoved,
        MapObjectsAboutToBeRemoved,
        MapObjectsAdded,
        MapObjectsChanged,
        MapObjectsRemoved,
        ObjectGroupChanged,
        TilesAboutToBeRemoved,
        TilesetChanged,
        WangSetAboutToBeAdded,
        WangSetAboutToBeRemoved,
        WangSetAdded,
        WangSetRemoved,
        WangSetChanged,
        WangColorAboutToBeRemoved,
    } type;

protected:
    ChangeEvent(Type type)
        : type(type)
    {}

    // not virtual, but protected to avoid calling at this level
    ~ChangeEvent()
    {}
};

class ObjectsChangeEvent : public ChangeEvent
{
public:
    enum ObjectProperty {
        ClassProperty           = 1 << 0,
    };

    ObjectsChangeEvent(const QList<Object *> &objects, int properties)
        : ChangeEvent(ObjectsChanged)
        , objects(objects)
        , properties(properties)
    {}

    QList<Object *> objects;
    int properties;
};

class MapChangeEvent : public ChangeEvent
{
public:
    MapChangeEvent(Map::Property property)
        : ChangeEvent(MapChanged)
        , property(property)
    {}

    Map::Property property;
};

class LayerChangeEvent : public ChangeEvent
{
public:
    enum LayerProperty {
        NameProperty            = 1 << 0,
        OpacityProperty         = 1 << 1,
        VisibleProperty         = 1 << 2,
        LockedProperty          = 1 << 3,
        OffsetProperty          = 1 << 4,
        ParallaxFactorProperty  = 1 << 5,
        TintColorProperty       = 1 << 6,
        PositionProperties      = OffsetProperty | ParallaxFactorProperty,
        AllProperties           = 0xFF
    };

    LayerChangeEvent(Layer *layer, int properties = AllProperties)
        : LayerChangeEvent(LayerChanged, layer, properties)
    {}

    Layer *layer;
    int properties;

protected:
    LayerChangeEvent(Type type, Layer *layer, int properties = AllProperties)
        : ChangeEvent(type)
        , layer(layer)
        , properties(properties)
    {}
};

class TileLayerChangeEvent : public LayerChangeEvent
{
public:
    enum TileLayerProperty {
        SizeProperty            = 1 << 7,
    };

    TileLayerChangeEvent(TileLayer *tileLayer, int properties)
        : LayerChangeEvent(TileLayerChanged, tileLayer, properties)
    {}

    TileLayer *tileLayer() const { return static_cast<TileLayer*>(layer); }
};

class ImageLayerChangeEvent : public LayerChangeEvent
{
public:
    enum TileLayerProperty {
        TransparentColorProperty    = 1 << 7,
        ImageSourceProperty         = 1 << 8,
        RepeatProperty              = 1 << 9,
    };

    ImageLayerChangeEvent(ImageLayer *imageLayer, int properties)
        : LayerChangeEvent(ImageLayerChanged, imageLayer, properties)
    {}

    ImageLayer *imageLayer() const { return static_cast<ImageLayer*>(layer); }
};

class ObjectGroupChangeEvent : public ChangeEvent
{
public:
    enum ObjectGroupProperty {
        ColorProperty           = 1 << 0,
        DrawOrderProperty       = 1 << 1,
    };

    ObjectGroupChangeEvent(ObjectGroup *objectGroup, int properties)
        : ChangeEvent(ObjectGroupChanged)
        , objectGroup(objectGroup)
        , properties(properties)
    {}

    ObjectGroup *objectGroup;
    int properties;
};

class MapObjectsEvent : public ChangeEvent
{
public:
    MapObjectsEvent(Type type, QList<MapObject *> mapObjects)
        : ChangeEvent(type)
        , mapObjects(std::move(mapObjects))
    {}

    QList<MapObject *> mapObjects;
};

class MapObjectsChangeEvent : public MapObjectsEvent
{
public:
    MapObjectsChangeEvent(MapObject *mapObject,
                          MapObject::ChangedProperties properties = MapObject::AllProperties)
        : MapObjectsChangeEvent(QList<MapObject *> { mapObject }, properties)
    {}

    MapObjectsChangeEvent(QList<MapObject *> mapObjects,
                          MapObject::ChangedProperties properties = MapObject::AllProperties)
        : MapObjectsEvent(MapObjectsChanged, std::move(mapObjects))
        , properties(properties)
    {}

    MapObject::ChangedProperties properties;
};

class MapObjectEvent : public ChangeEvent
{
public:
    MapObjectEvent(Type type, ObjectGroup *objectGroup, int index)
        : ChangeEvent(type)
        , objectGroup(objectGroup)
        , index(index)
    {}

    ObjectGroup *objectGroup;
    int index;
};

class TilesetChangeEvent : public ChangeEvent
{
public:
    TilesetChangeEvent(Tileset *tileset, Tileset::Property property)
        : ChangeEvent(TilesetChanged)
        , tileset(tileset)
        , property(property)
    {}

    Tileset *tileset;
    Tileset::Property property;
};

class TilesEvent : public ChangeEvent
{
public:
    TilesEvent(Type type, QList<Tile *> tiles)
        : ChangeEvent(type)
        , tiles(std::move(tiles))
    {}

    QList<Tile *> tiles;
};

class WangSetEvent : public ChangeEvent
{
public:
    WangSetEvent(Type type, Tileset *tileset, int index)
        : ChangeEvent(type)
        , tileset(tileset)
        , index(index)
    {}

    Tileset *tileset;
    int index;
};

class WangSetChangeEvent : public ChangeEvent
{
public:
    enum WangSetProperty {
        TypeProperty            = 1 << 0,
    };

    WangSetChangeEvent(WangSet *wangSet, int properties)
        : ChangeEvent(WangSetChanged)
        , wangSet(wangSet)
        , properties(properties)
    {}

    WangSet *wangSet;
    int properties;
};

class WangColorEvent : public ChangeEvent
{
public:
    WangColorEvent(Type type, WangSet *wangSet, int color)
        : ChangeEvent(type)
        , wangSet(wangSet)
        , color(color)
    {}

    WangSet *wangSet;
    int color;
};

} // namespace Tiled
