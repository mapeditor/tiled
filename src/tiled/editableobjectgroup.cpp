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
#include "editableasset.h"
#include "map.h"
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
    QList<QObject*> objects;
    for (MapObject *object : objectGroup()->objects())
        objects.append(EditableMapObject::get(asset(), object));
    return objects;
}

EditableMapObject *EditableObjectGroup::objectAt(int index)
{
    if (index < 0 || index >= objectCount()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Index out of range"));
        return nullptr;
    }

    auto mapObject = objectGroup()->objectAt(index);
    return EditableMapObject::get(asset(), mapObject);
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
    } else if (!checkReadOnly()) {
        objectGroup()->removeObjectAt(index);
        EditableMapObject::release(mapObject);
    }
}

void EditableObjectGroup::removeObject(EditableMapObject *editableMapObject)
{
    if (!editableMapObject) {
        ScriptManager::instance().throwNullArgError(0);
        return;
    }
    int index = objectGroup()->objects().indexOf(editableMapObject->mapObject());
    if (index == -1) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Object not found"));
        return;
    }

    removeObjectAt(index);
}

void EditableObjectGroup::insertObjectAt(int index, EditableMapObject *editableMapObject)
{
    if (!editableMapObject) {
        ScriptManager::instance().throwNullArgError(1);
        return;
    }
    if (index < 0 || index > objectCount()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Index out of range"));
        return;
    }

    if (!editableMapObject->isOwning()) {
        ScriptManager::instance().throwError(QCoreApplication::translate("Script Errors", "Object already part of an object layer"));
        return;
    }

    if (checkReadOnly())
        return;

    auto mapObject = editableMapObject->mapObject();

    // Avoid duplicate IDs by resetting when needed
    if (Map *map = objectGroup()->map()) {
        if (mapObject->id() != 0 && map->findObjectById(mapObject->id()))
            mapObject->resetId();
    }

    if (auto doc = document()) {
        AddRemoveMapObjects::Entry entry { mapObject, objectGroup() };
        entry.index = index;
        asset()->push(new AddMapObjects(doc, { entry }));
    } else {
        // ownership moves to the object group
        objectGroup()->insertObject(index, editableMapObject->attach(asset()));
    }
}

void EditableObjectGroup::addObject(EditableMapObject *editableMapObject)
{
    insertObjectAt(objectCount(), editableMapObject);
}

/**
 * This functions exists in addition to EditableLayer::get() because the asset
 * might also be an EditableTileset in the case of object groups.
 */
EditableObjectGroup *EditableObjectGroup::get(EditableAsset *asset, ObjectGroup *objectGroup)
{
    if (!objectGroup)
        return nullptr;

    if (auto editable = EditableLayer::find(objectGroup))
        return static_cast<EditableObjectGroup*>(editable);

    auto editable = new EditableObjectGroup(asset, objectGroup);
    editable->moveOwnershipToCpp();
    return editable;
}

void EditableObjectGroup::setColor(const QColor &color)
{
    if (auto doc = document()) {
        asset()->push(new ChangeObjectGroupColor(doc, { objectGroup() }, color));
    } else if (!checkReadOnly()) {
        objectGroup()->setColor(color);
    }
}

void EditableObjectGroup::setDrawOrder(DrawOrder drawOrder)
{
    if (auto doc = document()) {
        asset()->push(new ChangeObjectGroupDrawOrder(doc,
                                                     { objectGroup() },
                                                     static_cast<ObjectGroup::DrawOrder>(drawOrder)));
    } else if (!checkReadOnly()) {
        objectGroup()->setDrawOrder(static_cast<ObjectGroup::DrawOrder>(drawOrder));
    }
}

} // namespace Tiled

#include "moc_editableobjectgroup.cpp"
