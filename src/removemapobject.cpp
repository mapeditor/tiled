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

#include "removemapobject.h"

#include "mapdocument.h"
#include "mapobject.h"
#include "objectgroup.h"

using namespace Tiled;
using namespace Tiled::Internal;

RemoveMapObject::RemoveMapObject(MapDocument *mapDocument,
                                 MapObject *mapObject)
    : mMapDocument(mapDocument)
    , mMapObject(mapObject)
    , mObjectGroup(mapObject->objectGroup())
    , mOwnsObject(false)
{
    setText(QObject::tr("Remove Object"));
}

RemoveMapObject::~RemoveMapObject()
{
    if (mOwnsObject)
        delete mMapObject;
}

void RemoveMapObject::undo()
{
    mObjectGroup->addObject(mMapObject);
    mMapDocument->emitObjectAdded(mMapObject);
    mOwnsObject = false;
}

void RemoveMapObject::redo()
{
    mObjectGroup->removeObject(mMapObject);
    mMapDocument->emitObjectRemoved(mMapObject);
    mOwnsObject = true;
}
