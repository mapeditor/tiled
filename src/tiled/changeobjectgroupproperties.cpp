/*
 * changeobjectgroupproperties.cpp
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "changeobjectgroupproperties.h"

#include "mapdocument.h"
#include "objectgroup.h"
#include "mapobjectmodel.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

ChangeObjectGroupProperties::ChangeObjectGroupProperties(
        MapDocument *mapDocument,
        ObjectGroup *objectGroup,
        const QColor &newColor,
        ObjectGroup::DrawOrder newDrawOrder)
    : QUndoCommand(
        QCoreApplication::translate(
            "Undo Commands", "Change Object Layer Properties"))
    , mMapDocument(mapDocument)
    , mObjectGroup(objectGroup)
    , mUndoColor(objectGroup->color())
    , mRedoColor(newColor)
    , mUndoDrawOrder(objectGroup->drawOrder())
    , mRedoDrawOrder(newDrawOrder)
{
}

void ChangeObjectGroupProperties::redo()
{
    mObjectGroup->setColor(mRedoColor);
    mObjectGroup->setDrawOrder(mRedoDrawOrder);
    mMapDocument->emitObjectGroupChanged(mObjectGroup);
}

void ChangeObjectGroupProperties::undo()
{
    mObjectGroup->setColor(mUndoColor);
    mObjectGroup->setDrawOrder(mUndoDrawOrder);
    mMapDocument->emitObjectGroupChanged(mObjectGroup);
}
