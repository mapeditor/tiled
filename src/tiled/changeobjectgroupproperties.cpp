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

#include "changeevents.h"
#include "document.h"

#include <QCoreApplication>

using namespace Tiled;

ChangeObjectGroupProperties::ChangeObjectGroupProperties(Document *document,
                                                         ObjectGroup *objectGroup,
                                                         const QColor &newColor,
                                                         ObjectGroup::DrawOrder newDrawOrder)
    : QUndoCommand(
          QCoreApplication::translate(
              "Undo Commands", "Change Object Layer Properties"))
    , mDocument(document)
    , mObjectGroup(objectGroup)
    , mUndoColor(objectGroup->color())
    , mRedoColor(newColor)
    , mUndoDrawOrder(objectGroup->drawOrder())
    , mRedoDrawOrder(newDrawOrder)
{
}

void ChangeObjectGroupProperties::undo()
{
    int properties = 0;

    if (mObjectGroup->color() != mUndoColor) {
        mObjectGroup->setColor(mUndoColor);
        properties |= ObjectGroupChangeEvent::ColorProperty;
    }

    if (mObjectGroup->drawOrder() != mUndoDrawOrder) {
        mObjectGroup->setDrawOrder(mUndoDrawOrder);
        properties |= ObjectGroupChangeEvent::DrawOrderProperty;
    }

    emit mDocument->changed(ObjectGroupChangeEvent(mObjectGroup, properties));
}

void ChangeObjectGroupProperties::redo()
{
    int properties = 0;

    if (mObjectGroup->color() != mRedoColor) {
        mObjectGroup->setColor(mRedoColor);
        properties |= ObjectGroupChangeEvent::ColorProperty;
    }

    if (mObjectGroup->drawOrder() != mRedoDrawOrder) {
        mObjectGroup->setDrawOrder(mRedoDrawOrder);
        properties |= ObjectGroupChangeEvent::DrawOrderProperty;
    }

    emit mDocument->changed(ObjectGroupChangeEvent(mObjectGroup, properties));
}
