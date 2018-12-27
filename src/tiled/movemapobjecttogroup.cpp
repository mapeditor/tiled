/*
 * movemapobjecttogroup.cpp
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
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

#include "movemapobjecttogroup.h"

#include "mapdocument.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "mapobjectmodel.h"

#include <QCoreApplication>

using namespace Tiled;

MoveMapObjectToGroup::MoveMapObjectToGroup(MapDocument *mapDocument,
                                           MapObject *mapObject,
                                           ObjectGroup *objectGroup)
    : mMapDocument(mapDocument)
    , mMapObject(mapObject)
    , mOldObjectGroup(nullptr)
    , mNewObjectGroup(objectGroup)
    , mOldIndex(-1)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Move Object to Layer"));
}

void MoveMapObjectToGroup::undo()
{
    mMapDocument->mapObjectModel()->removeObject(mNewObjectGroup, mMapObject);
    mMapDocument->mapObjectModel()->insertObject(mOldObjectGroup, mOldIndex, mMapObject);
}

void MoveMapObjectToGroup::redo()
{
    mOldObjectGroup = mMapObject->objectGroup();
    mOldIndex = mOldObjectGroup->objects().indexOf(mMapObject);

    mMapDocument->mapObjectModel()->removeObject(mOldObjectGroup, mMapObject);
    mMapDocument->mapObjectModel()->insertObject(mNewObjectGroup, -1, mMapObject);
}
