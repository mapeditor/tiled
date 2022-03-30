/*
 * flipmapobjects.cpp
 * Copyright 2013-2022, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2017, Klimov Viktor <vitek.fomino@bk.ru>
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

#include "flipmapobjects.h"

#include "changeevents.h"
#include "document.h"
#include "mapobject.h"

#include <QCoreApplication>

using namespace Tiled;

FlipMapObjects::FlipMapObjects(Document *document,
                               const QList<MapObject *> &mapObjects,
                               FlipDirection flipDirection,
                               QPointF flipOrigin)
    : mDocument(document)
    , mMapObjects(mapObjects)
    , mFlipDirection(flipDirection)
    , mFlipOrigin(flipOrigin)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Flip %n Object(s)",
                                        nullptr, mapObjects.size()));

    mOldCellStates.reserve(mMapObjects.size());
    mNewCellStates.fill(true, mMapObjects.size());

    mOldRotationStates.reserve(mMapObjects.size());
    mNewRotationStates.fill(true, mMapObjects.size());

    for (MapObject *object : mMapObjects) {
        mOldCellStates.append(object->propertyChanged(MapObject::CellProperty));
        mOldRotationStates.append(object->propertyChanged(MapObject::RotationProperty));
    }
}

void FlipMapObjects::flip()
{
    for (int i = 0; i < mMapObjects.size(); ++i) {
        mMapObjects[i]->flip(mFlipDirection, mFlipOrigin);

        mMapObjects[i]->setPropertyChanged(MapObject::CellProperty, mNewCellStates[i]);
        mMapObjects[i]->setPropertyChanged(MapObject::RotationProperty, mNewRotationStates[i]);
    }

    mOldRotationStates.swap(mNewRotationStates);

    constexpr MapObject::ChangedProperties changedProperties {
        MapObject::CellProperty,
        MapObject::PositionProperty,
        MapObject::RotationProperty,
        MapObject::ShapeProperty,
    };
    emit mDocument->changed(MapObjectsChangeEvent(mMapObjects, changedProperties));
}
