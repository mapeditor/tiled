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

void TransformState::setPosition(QPointF position)
{
    if (mPosition != position) {
        mPosition = position;
        mChangedProperties |= MapObject::PositionProperty;
        mPropertiesChangedNow |= MapObject::PositionProperty;
    }
}

void TransformState::setSize(QSizeF size)
{
    if (mSize != size) {
        mSize = size;
        mChangedProperties |= MapObject::SizeProperty;
        mPropertiesChangedNow |= MapObject::SizeProperty;
    }
}

void TransformState::setPolygon(const QPolygonF &polygon)
{
    if (mPolygon != polygon) {
        mPolygon = polygon;
        mChangedProperties |= MapObject::ShapeProperty;
        mPropertiesChangedNow |= MapObject::ShapeProperty;
    }
}

void TransformState::setRotation(qreal rotation)
{
    if (mRotation != rotation) {
        mRotation = rotation;
        mChangedProperties |= MapObject::RotationProperty;
        mPropertiesChangedNow |= MapObject::RotationProperty;
    }
}

TransformMapObjects::TransformMapObjects(Document *document,
                                         QList<MapObject *> mapObjects,
                                         const QVector<TransformState> &states,
                                         QUndoCommand *parent)
    : ChangeValue<MapObject, TransformState>(document,
                                             std::move(mapObjects),
                                             states,
                                             parent)
{
    for (const TransformState &state : states)
        mChangedProperties |= state.propertiesChangedNow();

    if (mChangedProperties & MapObject::RotationProperty)
        setText(QCoreApplication::translate("Tiled::ObjectSelectionTool", "Rotate %n Object(s)", nullptr, states.size()));
    else if (mChangedProperties & (MapObject::SizeProperty | MapObject::ShapeProperty))
        setText(QCoreApplication::translate("Tiled::ObjectSelectionTool", "Resize %n Object(s)", nullptr, states.size()));
    else if (mChangedProperties & MapObject::PositionProperty)
        setText(QCoreApplication::translate("Tiled::ObjectSelectionTool", "Move %n Object(s)", nullptr, states.size()));
    else
        setText(QCoreApplication::translate("Undo Commands", "Transform %n Object(s)", nullptr, states.size()));
}

void TransformMapObjects::undo()
{
    ChangeValue<MapObject, TransformState>::undo();
    emit document()->changed(MapObjectsChangeEvent(objects(), mChangedProperties));
}

void TransformMapObjects::redo()
{
    ChangeValue<MapObject, TransformState>::redo();
    emit document()->changed(MapObjectsChangeEvent(objects(), mChangedProperties));
}

bool TransformMapObjects::mergeWith(const QUndoCommand *other)
{
    // Don't merge when the other command affects different properties
    auto o = static_cast<const TransformMapObjects*>(other);
    if (mChangedProperties != o->mChangedProperties)
        return false;

    return ChangeValue<MapObject, TransformState>::mergeWith(other);
}

TransformState TransformMapObjects::getValue(const MapObject *mapObject) const
{
    return TransformState(mapObject);
}

void TransformMapObjects::setValue(MapObject *mapObject, const TransformState &value) const
{
    mapObject->setPosition(value.position());
    mapObject->setSize(value.size());
    mapObject->setPolygon(value.polygon());
    mapObject->setRotation(value.rotation());
    mapObject->setChangedProperties(value.changedProperties());
}
