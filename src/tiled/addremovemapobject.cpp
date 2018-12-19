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

#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "mapobjectmodel.h"

#include <QCoreApplication>

#include "qtcompat_p.h"

using namespace Tiled;

AddRemoveMapObjects::AddRemoveMapObjects(MapDocument *mapDocument,
                                         const QVector<Entry> &entries,
                                         bool ownObjects,
                                         QUndoCommand *parent)
    : QUndoCommand(parent)
    , mMapDocument(mapDocument)
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

    for (const Entry &entry : qAsConst(entries))
        objects.append(entry.mapObject);

    return objects;
}


AddMapObjects::AddMapObjects(MapDocument *mapDocument, ObjectGroup *objectGroup,
                             MapObject *mapObject, QUndoCommand *parent)
    : AddRemoveMapObjects(mapDocument,
                          { Entry { mapObject, objectGroup } },
                          true,
                          parent)
{
    setText(QCoreApplication::translate("Undo Commands", "Add Object"));
}

AddMapObjects::AddMapObjects(MapDocument *mapDocument,
                             const QVector<AddRemoveMapObjects::Entry> &entries,
                             QUndoCommand *parent)
    : AddRemoveMapObjects(mapDocument,
                          entries,
                          true,
                          parent)
{
    setText(QCoreApplication::translate("Undo Commands", "Add Objects"));
}

void AddMapObjects::undo()
{
    // Deselecting all objects to be removed here avoids causing a selection
    // change for each individual object.
    mMapDocument->deselectObjects(objects(mEntries));

    auto model = mMapDocument->mapObjectModel();
    for (int i = mEntries.size() - 1; i >= 0; --i) {
        Entry &entry = mEntries[i];
        entry.index = model->removeObject(entry.objectGroup, entry.mapObject);
    }

    mOwnsObjects = true;

    QUndoCommand::undo(); // undo child commands
}

void AddMapObjects::redo()
{
    QUndoCommand::redo(); // redo child commands

    auto model = mMapDocument->mapObjectModel();
    for (const Entry &entry : qAsConst(mEntries))
        model->insertObject(entry.objectGroup, entry.index, entry.mapObject);

    mOwnsObjects = false;
}


RemoveMapObjects::RemoveMapObjects(MapDocument *mapDocument,
                                   MapObject *mapObject,
                                   QUndoCommand *parent)
    : AddRemoveMapObjects(mapDocument,
                          { Entry { mapObject, mapObject->objectGroup() } },
                          false,
                          parent)
{
    setText(QCoreApplication::translate("Undo Commands", "Remove Object"));
}

RemoveMapObjects::RemoveMapObjects(MapDocument *mapDocument,
                                   const QList<MapObject *> &mapObjects,
                                   QUndoCommand *parent)
    : AddRemoveMapObjects(mapDocument,
                          entries(mapObjects),
                          false,
                          parent)
{
    setText(QCoreApplication::translate("Undo Commands", "Remove Objects"));
}

void RemoveMapObjects::undo()
{
    auto model = mMapDocument->mapObjectModel();
    for (int i = mEntries.size() - 1; i >= 0; --i) {
        const Entry &entry = mEntries.at(i);
        model->insertObject(entry.objectGroup, entry.index, entry.mapObject);
    }

    mOwnsObjects = false;
}

void RemoveMapObjects::redo()
{
    // Deselecting all objects to be removed here avoids causing a selection
    // change for each individual object.
    mMapDocument->deselectObjects(objects(mEntries));

    auto model = mMapDocument->mapObjectModel();
    for (Entry &entry : mEntries)
        entry.index = model->removeObject(entry.objectGroup, entry.mapObject);

    mOwnsObjects = true;
}
