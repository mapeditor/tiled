/*
 * transformmapobjects.cpp
 * Copyright 2022, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "transformmapobjects.h"

#include "changeevents.h"
#include "document.h"

#include <QCoreApplication>

using namespace Tiled;

static constexpr MapObject::ChangedProperties transformProperties {
    MapObject::PositionProperty,
    MapObject::SizeProperty,
    MapObject::RotationProperty,
};

void TransformState::setPosition(QPointF position)
{
    if (mPosition != position) {
        mPosition = position;
        mChangedProperties |= MapObject::PositionProperty;
    }
}

void TransformState::setSize(QSizeF size)
{
    if (mSize != size) {
        mSize = size;
        mChangedProperties |= MapObject::SizeProperty;
    }
}

void TransformState::setRotation(qreal rotation)
{
    if (mRotation != rotation) {
        mRotation = rotation;
        mChangedProperties |= MapObject::RotationProperty;
    }
}

TransformMapObjects::TransformMapObjects(Document *document,
                                         QList<MapObject *> mapObjects,
                                         const QVector<TransformState> &states)
    : ChangeValue<MapObject, TransformState>(document,
                                             std::move(mapObjects),
                                             states)
{
    setText(QCoreApplication::translate("Undo Commands", "Transform Object"));
}

void TransformMapObjects::undo()
{
    ChangeValue<MapObject, TransformState>::undo();
    emit document()->changed(MapObjectsChangeEvent(objects(), transformProperties));
}

void TransformMapObjects::redo()
{
    ChangeValue<MapObject, TransformState>::undo();
    emit document()->changed(MapObjectsChangeEvent(objects(), transformProperties));
}

TransformState TransformMapObjects::getValue(const MapObject *mapObject) const
{
    return TransformState(mapObject);
}

void TransformMapObjects::setValue(MapObject *mapObject, const TransformState &value) const
{
    mapObject->setPosition(value.position());
    mapObject->setSize(value.size());
    mapObject->setRotation(value.rotation());
    mapObject->setChangedProperties(value.changedProperties());
}
