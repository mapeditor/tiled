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

EditableMapObject::EditableMapObject(const QString &name,
                                     QObject *parent)
    : EditableObject(nullptr, new MapObject(name), parent)
{
    mDetachedMapObject.reset(mapObject());
}

EditableMapObject::EditableMapObject(EditableMap *map,
                                     MapObject *mapObject,
                                     QObject *parent)
    : EditableObject(map, mapObject, parent)
{
}

bool EditableMapObject::isSelected() const
{
    if (auto m = map())
        if (auto doc = m->mapDocument())
            return doc->selectedObjects().contains(mapObject());
    return false;
}

EditableObjectGroup *EditableMapObject::layer() const
{
    if (map())
        return static_cast<EditableObjectGroup*>(map()->editableLayer(mapObject()->objectGroup()));
    else
        // todo: what to do for objects that are part of detached layers?
        ;

    return nullptr;
}

EditableMap *EditableMapObject::map() const
{
    return static_cast<EditableMap*>(asset());
}

void EditableMapObject::detach()
{
    Q_ASSERT(map());
    Q_ASSERT(map()->mEditableMapObjects.contains(mapObject()));

    map()->mEditableMapObjects.remove(mapObject());
    setAsset(nullptr);

    mDetachedMapObject.reset(mapObject()->clone());
    setObject(mDetachedMapObject.get());
}

void EditableMapObject::attach(EditableMap *map)
{
    Q_ASSERT(!asset() && map);
    Q_ASSERT(!map->mEditableMapObjects.contains(mapObject()));

    setAsset(map);
    map->mEditableMapObjects.insert(mapObject(), this);
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
    if (asset()) {
        asset()->push(new MoveMapObject(map()->mapDocument(), mapObject(),
                                        pos, mapObject()->position()));
    } else {
        mapObject()->setPosition(pos);
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

void EditableMapObject::setSelected(bool selected)
{
    auto document = map() ? map()->mapDocument() : nullptr;
    if (!document)
        return;

    if (selected) {
        if (!document->selectedObjects().contains(mapObject())) {
            auto objects = document->selectedObjects();
            objects.append(mapObject());
            document->setSelectedObjects(objects);
        }
    } else {
        int index = document->selectedObjects().indexOf(mapObject());
        if (index != -1) {
            auto objects = document->selectedObjects();
            objects.removeAt(index);
            document->setSelectedObjects(objects);
        }
    }
}

void EditableMapObject::setMapObjectProperty(MapObject::Property property,
                                             const QVariant &value)
{
    if (asset()) {
        asset()->push(new ChangeMapObject(map()->mapDocument(), mapObject(),
                                          property, value));
    } else {
        mapObject()->setMapObjectProperty(property, value);
    }
}

} // namespace Tiled
