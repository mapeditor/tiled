/*
 * addremovemapobject.cpp
 * Copyright 2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "addremovemapobject.h"

#include "changeevents.h"
#include "document.h"
#include "map.h"
#include "mapobject.h"
#include "objectgroup.h"

#include <QCoreApplication>

#include "qtcompat_p.h"

using namespace Tiled;

AddRemoveMapObjects::AddRemoveMapObjects(Document *document,
                                         const QVector<Entry> &entries,
                                         bool ownObjects,
                                         QUndoCommand *parent)
    : QUndoCommand(parent)
    , mDocument(document)
    , mEntries(entries)
    , mOwnsObjects(ownObjects)
{
}

AddRemoveMapObjects::~AddRemoveMapObjects()
{
    if (mOwnsObjects)
        for (const Entry &entry : qAsConst(mEntries))
            delete entry.mapObject;
}

void AddRemoveMapObjects::releaseObjects()
{
    mOwnsObjects = false;
}

QVector<AddRemoveMapObjects::Entry> AddRemoveMapObjects::entries(const QList<MapObject *> &objects)
{
    QVector<Entry> entries;
    entries.reserve(objects.size());

    for (MapObject *object : objects)
        entries.append(Entry { object, object->objectGroup() });

    return entries;
}

QList<MapObject *> AddRemoveMapObjects::objects(const QVector<Entry> &entries)
{
    QList<MapObject*> objects;
    objects.reserve(entries.size());

    for (const Entry &entry : entries)
        objects.append(entry.mapObject);

    return objects;
}


AddMapObjects::AddMapObjects(Document *document, ObjectGroup *objectGroup,
                             MapObject *mapObject, QUndoCommand *parent)
    : AddRemoveMapObjects(document,
                          { Entry { mapObject, objectGroup } },
                          true,
                          parent)
{
    setText(QCoreApplication::translate("Undo Commands", "Add Object"));
}

AddMapObjects::AddMapObjects(Document *document,
                             const QVector<AddRemoveMapObjects::Entry> &entries,
                             QUndoCommand *parent)
    : AddRemoveMapObjects(document,
                          entries,
                          true,
                          parent)
{
    setText(QCoreApplication::translate("Undo Commands", "Add Objects"));
}

void AddMapObjects::undo()
{
    MapObjectsEvent mapObjectsEvent { ChangeEvent::MapObjectsAboutToBeRemoved, objects(mEntries) };

    emit mDocument->changed(mapObjectsEvent);

    for (int i = mEntries.size() - 1; i >= 0; --i) {
        Entry &entry = mEntries[i];
        emit mDocument->changed(MapObjectEvent(ChangeEvent::MapObjectAboutToBeRemoved, entry.objectGroup, entry.index));
        entry.objectGroup->removeObjectAt(entry.index);
        emit mDocument->changed(MapObjectEvent(ChangeEvent::MapObjectRemoved, entry.objectGroup, entry.index));
    }

    mapObjectsEvent.type = ChangeEvent::MapObjectsRemoved;
    emit mDocument->changed(mapObjectsEvent);

    mOwnsObjects = true;

    QUndoCommand::undo(); // undo child commands
}

void AddMapObjects::redo()
{
    QUndoCommand::redo(); // redo child commands

    for (Entry &entry : mEntries) {
        if (entry.index == -1)
            entry.index = entry.objectGroup->objectCount();

        emit mDocument->changed(MapObjectEvent(ChangeEvent::MapObjectAboutToBeAdded, entry.objectGroup, entry.index));
        entry.objectGroup->insertObject(entry.index, entry.mapObject);
        emit mDocument->changed(MapObjectEvent(ChangeEvent::MapObjectAdded, entry.objectGroup, entry.index));
    }

    emit mDocument->changed(MapObjectsEvent(ChangeEvent::MapObjectsAdded, objects(mEntries)));

    mOwnsObjects = false;
}


RemoveMapObjects::RemoveMapObjects(Document *document,
                                   MapObject *mapObject,
                                   QUndoCommand *parent)
    : AddRemoveMapObjects(document,
                          { Entry { mapObject, mapObject->objectGroup() } },
                          false,
                          parent)
{
    setText(QCoreApplication::translate("Undo Commands", "Remove Object"));
}

RemoveMapObjects::RemoveMapObjects(Document *document,
                                   const QList<MapObject *> &mapObjects,
                                   QUndoCommand *parent)
    : AddRemoveMapObjects(document,
                          entries(mapObjects),
                          false,
                          parent)
{
    setText(QCoreApplication::translate("Undo Commands", "Remove Objects"));
}

void RemoveMapObjects::undo()
{
    for (int i = mEntries.size() - 1; i >= 0; --i) {
        const Entry &entry = mEntries.at(i);
        emit mDocument->changed(MapObjectEvent(ChangeEvent::MapObjectAboutToBeAdded, entry.objectGroup, entry.index));
        entry.objectGroup->insertObject(entry.index, entry.mapObject);
        emit mDocument->changed(MapObjectEvent(ChangeEvent::MapObjectAdded, entry.objectGroup, entry.index));
    }

    emit mDocument->changed(MapObjectsEvent(ChangeEvent::MapObjectsAdded, objects(mEntries)));

    mOwnsObjects = false;
}

void RemoveMapObjects::redo()
{
    emit mDocument->changed(MapObjectsEvent(ChangeEvent::MapObjectsAboutToBeRemoved, objects(mEntries)));

    for (Entry &entry : mEntries) {
        if (entry.index == -1)
            entry.index = entry.objectGroup->objects().indexOf(entry.mapObject);

        emit mDocument->changed(MapObjectEvent(ChangeEvent::MapObjectAboutToBeRemoved, entry.objectGroup, entry.index));
        entry.objectGroup->removeObjectAt(entry.index);
        emit mDocument->changed(MapObjectEvent(ChangeEvent::MapObjectRemoved, entry.objectGroup, entry.index));
    }

    mOwnsObjects = true;
}
