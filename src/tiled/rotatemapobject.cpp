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

#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectmodel.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

RotateMapObject::RotateMapObject(MapDocument *mapDocument,
                MapObject *mapObject,
                qreal oldRotation)
    : mMapDocument(mapDocument)
    , mMapObject(mapObject)
    , mOldRotation(oldRotation)
    , mNewRotation(mapObject->rotation())
{
    setText(QCoreApplication::translate("Undo Commands", "Rotate Object"));
}

void RotateMapObject::undo()
{
    mMapDocument->mapObjectModel()->setObjectRotation(mMapObject, mOldRotation);
}

void RotateMapObject::redo()
{
    mMapDocument->mapObjectModel()->setObjectRotation(mMapObject, mNewRotation);
}
