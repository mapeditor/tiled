/*
 * movemapobject.cpp
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

#include "movemapobject.h"

#include "changeevents.h"
#include "document.h"
#include "mapobject.h"

#include <QCoreApplication>

using namespace Tiled;

MoveMapObject::MoveMapObject(Document *document,
                             MapObject *mapObject,
                             const QPointF &oldPos,
                             QUndoCommand *parent)
    : MoveMapObject(document,
                    mapObject,
                    mapObject->position(),
                    oldPos,
                    parent)
{
}

MoveMapObject::MoveMapObject(Document *document,
                             MapObject *mapObject,
                             const QPointF &newPos,
                             const QPointF &oldPos,
                             QUndoCommand *parent)
    : QUndoCommand(parent)
    , mDocument(document)
    , mMapObject(mapObject)
    , mOldPos(oldPos)
    , mNewPos(newPos)
{
    setText(QCoreApplication::translate("Undo Commands", "Move Object"));
}

void MoveMapObject::undo()
{
    mMapObject->setPosition(mOldPos);
    emit mDocument->changed(MapObjectsChangeEvent(mMapObject, MapObject::PositionProperty));
}

void MoveMapObject::redo()
{
    mMapObject->setPosition(mNewPos);
    emit mDocument->changed(MapObjectsChangeEvent(mMapObject, MapObject::PositionProperty));
}
