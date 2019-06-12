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

#include "mapobject.h"

#include <QList>

namespace Tiled {

class ChangeEvent
{
public:
    enum Type {
        MapObjectsChanged,
    } type;

protected:
    ChangeEvent(Type type)
        : type(type)
    {}
};

class MapObjectsChangeEvent : public ChangeEvent
{
public:
    MapObjectsChangeEvent(MapObject *mapObject,
                          MapObject::ChangedProperties properties = MapObject::AllProperties)
        : MapObjectsChangeEvent(QList<MapObject *> { mapObject }, properties)
    {}

    MapObjectsChangeEvent(QList<MapObject *> mapObjects,
                          MapObject::ChangedProperties properties = MapObject::AllProperties)
        : ChangeEvent(MapObjectsChanged)
        , mapObjects(std::move(mapObjects))
        , properties(properties)
    {}

    QList<MapObject *> mapObjects;
    MapObject::ChangedProperties properties;
};

} // namespace Tiled
