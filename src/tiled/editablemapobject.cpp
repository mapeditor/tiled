/*
 * editablemapobject.cpp
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editablemapobject.h"

#include "changemapobject.h"
#include "editablemap.h"
#include "editableobjectgroup.h"
#include "movemapobject.h"

namespace Tiled {

EditableMapObject::EditableMapObject(EditableMap *map,
                                     MapObject *mapObject,
                                     QObject *parent)
    : QObject(parent)
    , mMap(map)
    , mMapObject(mapObject)
{
}

EditableObjectGroup *EditableMapObject::layer() const
{
    if (mMap)
        return static_cast<EditableObjectGroup*>(mMap->editableLayer(mMapObject->objectGroup()));
    else
        // todo: what to do for objects that are part of detached layers?
        ;

    return nullptr;
}

bool EditableMapObject::isReadOnly() const
{
    return mMap && mMap->isReadOnly();
}

void EditableMapObject::detach()
{
    Q_ASSERT(mMap);
    Q_ASSERT(mMap->mEditableMapObjects.contains(mapObject()));

    mMap->mEditableMapObjects.remove(mapObject());
    mMap = nullptr;

    mDetachedMapObject.reset(mMapObject->clone());
    mMapObject = mDetachedMapObject.get();
}

void EditableMapObject::attach(EditableMap *map)
{
    Q_ASSERT(!mMap && map);
    Q_ASSERT(!map->mEditableMapObjects.contains(mapObject()));

    mMap = map;
    mMap->mEditableMapObjects.insert(mapObject(), this);
    mDetachedMapObject.release();
}

void EditableMapObject::setName(QString name)
{
    setMapObjectProperty(MapObject::NameProperty, name);
}

void EditableMapObject::setType(QString type)
{
    setMapObjectProperty(MapObject::TypeProperty, type);
}

void EditableMapObject::setPos(QPointF pos)
{
    if (mMap) {
        mMap->push(new MoveMapObject(mMap->mapDocument(), mMapObject,
                                     pos, mMapObject->position()));
    } else {
        mMapObject->setPosition(pos);
    }
}

void EditableMapObject::setSize(QSizeF size)
{
    setMapObjectProperty(MapObject::SizeProperty, size);
}

void EditableMapObject::setRotation(qreal rotation)
{
    setMapObjectProperty(MapObject::RotationProperty, rotation);
}

void EditableMapObject::setVisible(bool visible)
{
    setMapObjectProperty(MapObject::VisibleProperty, visible);
}

void EditableMapObject::setMapObjectProperty(MapObject::Property property,
                                             const QVariant &value)
{
    if (mMap) {
        mMap->push(new ChangeMapObject(mMap->mapDocument(), mMapObject,
                                       property, value));
    } else {
        mMapObject->setMapObjectProperty(property, value);
    }
}

} // namespace Tiled
