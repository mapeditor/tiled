/*
 * resizemapobject.cpp
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "resizemapobject.h"

#include "changeevents.h"
#include "document.h"
#include "mapobject.h"

#include <QCoreApplication>

using namespace Tiled;

ResizeMapObject::ResizeMapObject(Document *document,
                                 MapObject *mapObject,
                                 const QSizeF &oldSize)
    : ResizeMapObject(document,
                      mapObject,
                      mapObject->size(),
                      oldSize)
{
}

ResizeMapObject::ResizeMapObject(Document *document,
                                 MapObject *mapObject,
                                 const QSizeF &newSize,
                                 const QSizeF &oldSize)
    : mDocument(document)
    , mMapObject(mapObject)
    , mOldSize(oldSize)
    , mNewSize(newSize)
    , mOldChangeState(mapObject->propertyChanged(MapObject::SizeProperty))
{
    setText(QCoreApplication::translate("Undo Commands", "Resize Object"));
}

void ResizeMapObject::undo()
{
    mMapObject->setSize(mOldSize);
    mMapObject->setPropertyChanged(MapObject::SizeProperty, mOldChangeState);

    emit mDocument->changed(MapObjectsChangeEvent(mMapObject, MapObject::SizeProperty));
}

void ResizeMapObject::redo()
{
    mMapObject->setSize(mNewSize);
    mMapObject->setPropertyChanged(MapObject::SizeProperty);

    emit mDocument->changed(MapObjectsChangeEvent(mMapObject, MapObject::SizeProperty));
}
