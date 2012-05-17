/*
 * changemapobject.cpp
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

#include "changemapobject.h"

#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectmodel.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

ChangeMapObject::ChangeMapObject(MapDocument *mapDocument,
                                 MapObject *mapObject,
                                 const QString &name,
                                 const QString &type)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Object"))
    , mMapDocument(mapDocument)
    , mMapObject(mapObject)
    , mName(name)
    , mType(type)
{
}

void ChangeMapObject::undo()
{
    swap();
}

void ChangeMapObject::redo()
{
    swap();
}

void ChangeMapObject::swap()
{
    const QString name = mMapObject->name();
    const QString type = mMapObject->type();

    mMapDocument->mapObjectModel()->setObjectName(mMapObject, mName);
    mMapDocument->mapObjectModel()->setObjectType(mMapObject, mType);

    mName = name;
    mType = type;
}
