/*
 * changeproperties.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "changeproperties.h"

#include "mapdocument.h"

#include <QCoreApplication>

using namespace Tiled;
using namespace Tiled::Internal;

ChangeProperties::ChangeProperties(MapDocument *mapDocument,
                                   const QString &kind,
                                   Object *object,
                                   const Properties &newProperties)
    : mMapDocument(mapDocument)
    , mObject(object)
    , mNewProperties(newProperties)
{
    setText(QCoreApplication::translate("Undo Commands",
                                        "Change %1 Properties").arg(kind));
}

void ChangeProperties::redo()
{
    swapProperties();
}

void ChangeProperties::undo()
{
    swapProperties();
}

void ChangeProperties::swapProperties()
{
    const Properties oldProperties = mObject->properties();
    mMapDocument->setProperties(mObject, mNewProperties);
    mNewProperties = oldProperties;
}


SetProperty::SetProperty(MapDocument *mapDocument,
                         Object *object,
                         const QString &name,
                         const QString &value,
                         QUndoCommand *parent)
    : QUndoCommand(parent)
    , mMapDocument(mapDocument)
    , mObject(object)
    , mName(name)
    , mValue(value)
    , mPreviousValue(object->property(name))
    , mPropertyExisted(object->hasProperty(name))
{
    if (mPropertyExisted)
        setText(QCoreApplication::translate("Undo Commands", "Set Property"));
    else
        setText(QCoreApplication::translate("Undo Commands", "Add Property"));
}

void SetProperty::undo()
{
    if (mPropertyExisted)
        mMapDocument->setProperty(mObject, mName, mPreviousValue);
    else
        mMapDocument->removeProperty(mObject, mName);
}

void SetProperty::redo()
{
    mMapDocument->setProperty(mObject, mName, mValue);
}


RemoveProperty::RemoveProperty(MapDocument *mapDocument,
                               Object *object,
                               const QString &name,
                               QUndoCommand *parent)
    : QUndoCommand(parent)
    , mMapDocument(mapDocument)
    , mObject(object)
    , mName(name)
    , mPreviousValue(object->property(name))
{
    Q_ASSERT(object->hasProperty(name));

    setText(QCoreApplication::translate("Undo Commands", "Remove Property"));
}

void RemoveProperty::undo()
{
    mMapDocument->setProperty(mObject, mName, mPreviousValue);
}

void RemoveProperty::redo()
{
    mMapDocument->removeProperty(mObject, mName);
}


RenameProperty::RenameProperty(MapDocument *mapDocument,
                               Object *object,
                               const QString &oldName,
                               const QString &newName)
{
    setText(QCoreApplication::translate("Undo Commands", "Rename Property"));

    const QString value = object->property(oldName);

    new RemoveProperty(mapDocument, object, oldName, this);
    new SetProperty(mapDocument, object, newName, value, this);
}
