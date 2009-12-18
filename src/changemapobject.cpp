/*
 * Tiled Map Editor (Qt)
 * Copyright 2009 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "changemapobject.h"

#include "mapdocument.h"
#include "mapobject.h"

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

    mMapObject->setName(mName);
    mMapObject->setType(mType);
    mMapDocument->emitObjectChanged(mMapObject);

    mName = name;
    mType = type;
}
