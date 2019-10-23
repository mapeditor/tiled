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
#include "editablemanager.h"
#include "editablemap.h"
#include "scriptmanager.h"

#include <QCoreApplication>

namespace Tiled {

EditableObjectGroup::EditableObjectGroup(const QString &name, QObject *parent)
    : EditableLayer(std::unique_ptr<Layer>(new ObjectGroup(name)), parent)
{
}

EditableObjectGroup::EditableObjectGroup(EditableAsset *asset,
                                         ObjectGroup *objectGroup,
                                         QObject *parent)
    : EditableLayer(asset, objectGroup, parent)
{
}

QList<QObject *> EditableObjectGroup::objects()
{
    auto &editableManager = EditableManager::instance();
    QList<QObject*> objects;
    for (MapObject *object : objectGroup()->objects())
        objects.append(editableManager.editableMapObject(asset(), object));
    return objects;
}

EditableMapObject *EditableObjectGroup::objectAt(int index)
{
    if (index < 0 || index >= objectCount()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Index out of range"));
        return nullptr;
    }

    auto mapObject = objectGroup()->objectAt(index);
    return EditableManager::instance().editableMapObject(asset(), mapObject);
}

void EditableObjectGroup::removeObjectAt(int index)
{
    if (index < 0 || index >= objectCount()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Index out of range"));
        return;
    }

    auto mapObject = objectGroup()->objectAt(index);

    if (auto doc = document()) {
        asset()->push(new RemoveMapObjects(doc, mapObject));
    } else {
        objectGroup()->removeObjectAt(index);
        EditableManager::instance().release(mapObject);
    }
}

void EditableObjectGroup::removeObject(EditableMapObject *editableMapObject)
{
    int index = objectGroup()->objects().indexOf(editableMapObject->mapObject());
    if (index == -1) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Object not found"));
        return;
    }

    removeObjectAt(index);
}

void EditableObjectGroup::insertObjectAt(int index, EditableMapObject *editableMapObject)
{
    if (index < 0 || index > objectCount()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Index out of range"));
        return;
    }

    if (editableMapObject->mapObject()->objectGroup()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Object already part of an object layer"));
        return;
    }

    if (auto doc = document()) {
        asset()->push(new AddMapObjects(doc,
                                        objectGroup(),
                                        editableMapObject->mapObject()));
    } else {
        objectGroup()->insertObject(index, editableMapObject->mapObject());
        editableMapObject->release();   // now owned by the object group
    }
}

void EditableObjectGroup::addObject(EditableMapObject *editableMapObject)
{
    insertObjectAt(objectCount(), editableMapObject);
}

void EditableObjectGroup::setColor(const QColor &color)
{
    if (auto doc = document()) {
        asset()->push(new ChangeObjectGroupProperties(doc,
                                                      objectGroup(),
                                                      color,
                                                      objectGroup()->drawOrder()));
    } else {
        objectGroup()->setColor(color);
    }
}

} // namespace Tiled
