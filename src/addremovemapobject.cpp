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

#include "addremovemapobject.h"

#include "mapdocument.h"
#include "mapobject.h"
#include "objectgroup.h"

using namespace Tiled;
using namespace Tiled::Internal;

AddRemoveMapObject::AddRemoveMapObject(MapDocument *mapDocument,
                                       ObjectGroup *objectGroup,
                                       MapObject *mapObject,
                                       bool ownObject)
    : mMapDocument(mapDocument)
    , mMapObject(mapObject)
    , mObjectGroup(objectGroup)
    , mIndex(-1)
    , mOwnsObject(ownObject)
{
}

AddRemoveMapObject::~AddRemoveMapObject()
{
    if (mOwnsObject)
        delete mMapObject;
}

void AddRemoveMapObject::addObject()
{
    if (mIndex == -1)
        mObjectGroup->addObject(mMapObject);
    else
        mObjectGroup->insertObject(mIndex, mMapObject);

    mMapDocument->emitObjectAdded(mMapObject);
    mOwnsObject = false;
}

void AddRemoveMapObject::removeObject()
{
    mIndex = mObjectGroup->removeObject(mMapObject);
    mMapDocument->emitObjectRemoved(mMapObject);
    mOwnsObject = true;
}


RemoveMapObject::RemoveMapObject(MapDocument *mapDocument,
                                 MapObject *mapObject)
    : AddRemoveMapObject(mapDocument,
                         mapObject->objectGroup(),
                         mapObject,
                         false)
{
    setText(QObject::tr("Remove Object"));
}
