/*
 * rotatemapobject.cpp
 * Copyright 2012, Przemysław Grzywacz <nexather@gmail.com>
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

#include "rotatemapobject.h"

#include "changeevents.h"
#include "document.h"
#include "mapobject.h"

#include <QCoreApplication>

using namespace Tiled;

RotateMapObject::RotateMapObject(Document *document,
                                 MapObject *mapObject,
                                 qreal oldRotation)
    : RotateMapObject(document,
                      mapObject,
                      mapObject->rotation(),
                      oldRotation)
{
}

RotateMapObject::RotateMapObject(Document *document,
                                 MapObject *mapObject,
                                 qreal newRotation,
                                 qreal oldRotation)
    : mDocument(document)
    , mMapObject(mapObject)
    , mOldRotation(oldRotation)
    , mNewRotation(newRotation)
    , mOldChangeState(mapObject->propertyChanged(MapObject::RotationProperty))
{
    setText(QCoreApplication::translate("Undo Commands", "Rotate Object"));
}

void RotateMapObject::undo()
{
    mMapObject->setRotation(mOldRotation);
    mMapObject->setPropertyChanged(MapObject::RotationProperty, mOldChangeState);

    emit mDocument->changed(MapObjectsChangeEvent(mMapObject, MapObject::RotationProperty));
}

void RotateMapObject::redo()
{
    mMapObject->setRotation(mNewRotation);
    mMapObject->setPropertyChanged(MapObject::RotationProperty);

    emit mDocument->changed(MapObjectsChangeEvent(mMapObject, MapObject::RotationProperty));
}
