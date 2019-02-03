/*
 * editableobjectgroup.cpp
 * Copyright 2018, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "editableobjectgroup.h"

#include "addremovemapobject.h"
#include "changeobjectgroupproperties.h"
#include "editablemap.h"
#include "scriptmanager.h"

namespace Tiled {

EditableObjectGroup::EditableObjectGroup(const QString &name, QObject *parent)
    : EditableLayer(std::unique_ptr<Layer>(new ObjectGroup(name)), parent)
{
}

EditableObjectGroup::EditableObjectGroup(EditableMap *map,
                                         ObjectGroup *objectGroup,
                                         QObject *parent)
    : EditableLayer(map, objectGroup, parent)
{
}

QList<QObject *> EditableObjectGroup::objects()
{
    if (!map()) // todo: unsupported for stand-alone object groups at the moment...
        return QList<QObject*>();

    QList<QObject*> objects;
    for (MapObject *object : objectGroup()->objects())
        objects.append(map()->editableMapObject(object));
    return objects;
}

EditableMapObject *EditableObjectGroup::objectAt(int index)
{
    if (index < 0 || index >= objectCount()) {
        ScriptManager::instance().throwError(tr("Index out of range"));
        return nullptr;
    }

    auto mapObject = objectGroup()->objectAt(index);

    if (map()) {
        return map()->editableMapObject(mapObject);
    } else {
        // todo: what's going to ensure this object doesn't become roaming?
        // todo: what about avoiding the creation of multiple instances pointing to the same object?
        return new EditableMapObject(nullptr, mapObject);
    }
}

void EditableObjectGroup::removeObjectAt(int index)
{
    if (index < 0 || index >= objectCount()) {
        ScriptManager::instance().throwError(tr("Index out of range"));
        return;
    }

    auto mapObject = objectGroup()->objectAt(index);

    if (map()) {
        map()->push(new RemoveMapObjects(map()->mapDocument(), mapObject));
    } else {
        objectGroup()->removeObjectAt(index);
        // todo: if there is still a EditableMapObject instance pointing to this object, it should not be deleted
        delete mapObject;
    }
}

void EditableObjectGroup::removeObject(EditableMapObject *editableMapObject)
{
    int index = objectGroup()->objects().indexOf(editableMapObject->mapObject());
    if (index == -1) {
        ScriptManager::instance().throwError(tr("Object not found"));
        return;
    }

    removeObjectAt(index);
}

void EditableObjectGroup::insertObjectAt(int index, EditableMapObject *editableMapObject)
{
    if (index < 0 || index > objectCount()) {
        ScriptManager::instance().throwError(tr("Index out of range"));
        return;
    }

    if (editableMapObject->mapObject()->objectGroup()) {
        ScriptManager::instance().throwError(tr("Object already part of an object layer"));
        return;
    }

    if (map()) {
        if (map()->push(new AddMapObjects(map()->mapDocument(),
                                          objectGroup(),
                                          editableMapObject->mapObject()))) {
            editableMapObject->attach(map());
        }
    } else {
        // todo: when this ObjectGroup is added to a map later, who is going to
        // attach the EditableMapObject, which is not referenced anywhere?
        objectGroup()->insertObject(index, editableMapObject->mapObject());
    }
}

void EditableObjectGroup::addObject(EditableMapObject *editableMapObject)
{
    insertObjectAt(objectCount(), editableMapObject);
}

void EditableObjectGroup::setColor(const QColor &color)
{
    if (map()) {
        map()->push(new ChangeObjectGroupProperties(map()->mapDocument(),
                                                    objectGroup(),
                                                    color,
                                                    objectGroup()->drawOrder()));
    } else {
        objectGroup()->setColor(color);
    }
}

} // namespace Tiled
