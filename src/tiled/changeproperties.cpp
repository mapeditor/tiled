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
                         const QList<Object*> &objects,
                         const QString &name,
                         const QVariant &value,
                         QUndoCommand *parent)
    : QUndoCommand(parent)
    , mMapDocument(mapDocument)
    , mObjects(objects)
    , mName(name)
    , mValue(value)
{
    for (Object *obj : objects) {
        ObjectProperty prop;
        prop.existed = obj->hasProperty(mName);
        prop.previousValue = obj->property(mName);
        mProperties.append(prop);
    }

    if (mObjects.size() > 1 || mObjects.at(0)->hasProperty(mName))
        setText(QCoreApplication::translate("Undo Commands", "Set Property"));
    else
        setText(QCoreApplication::translate("Undo Commands", "Add Property"));
}

void SetProperty::undo()
{
    for (int i = 0; i < mObjects.size(); ++i) {
        if (mProperties.at(i).existed)
            mMapDocument->setProperty(mObjects.at(i), mName, mProperties.at(i).previousValue);
        else
            mMapDocument->removeProperty(mObjects.at(i), mName);
    }
}

void SetProperty::redo()
{
    const QList<Object*> &objects = mObjects;
    for (Object *obj : objects)
        mMapDocument->setProperty(obj, mName, mValue);
}


RemoveProperty::RemoveProperty(MapDocument *mapDocument,
                               const QList<Object*> &objects,
                               const QString &name,
                               QUndoCommand *parent)
    : QUndoCommand(parent)
    , mMapDocument(mapDocument)
    , mObjects(objects)
    , mName(name)
{
    for (Object *obj : objects)
        mPreviousValues.append(obj->property(mName));

    setText(QCoreApplication::translate("Undo Commands", "Remove Property"));
}

void RemoveProperty::undo()
{
    for (int i = 0; i < mObjects.size(); ++i)
        mMapDocument->setProperty(mObjects.at(i), mName, mPreviousValues.at(i));
}

void RemoveProperty::redo()
{
    const QList<Object*> &objects = mObjects;
    for (Object *obj : objects)
        mMapDocument->removeProperty(obj, mName);
}


RenameProperty::RenameProperty(MapDocument *mapDocument,
                               const QList<Object*> &objects,
                               const QString &oldName,
                               const QString &newName)
{
    setText(QCoreApplication::translate("Undo Commands", "Rename Property"));

    // Remove the old name from all objects
    new RemoveProperty(mapDocument, objects, oldName, this);

    // Different objects may have different values for the same property,
    // or may not have a value at all.
    for (Object *object : objects) {
        if (!object->hasProperty(oldName))
            continue;

        const QList<Object*> objects { object };
        const QVariant value = object->property(oldName);

        new SetProperty(mapDocument, objects, newName, value, this);
    }
}
