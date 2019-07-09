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

#include "addremovemapobject.h"

#include <QCoreApplication>

using namespace Tiled;

MoveMapObjectToGroup::MoveMapObjectToGroup(Document *document,
                                           MapObject *mapObject,
                                           ObjectGroup *objectGroup)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Move Object to Layer"));

    mRemoveMapObject = new RemoveMapObjects(document, mapObject, this);
    mAddMapObject = new AddMapObjects(document, objectGroup, mapObject, this);
}

MoveMapObjectToGroup::~MoveMapObjectToGroup()
{
    // Make sure the object doesn't get deleted (we don't own it, we just moved it)
    mRemoveMapObject->releaseObjects();
    mAddMapObject->releaseObjects();
}
