/*
 * changemapobject.cpp
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

#include "changemapobject.h"

#include "mapdocument.h"
#include "mapobjectmodel.h"
#include "objecttemplate.h"

#include <QCoreApplication>

#include "changeevents.h"
#include "qtcompat_p.h"

using namespace Tiled;

ChangeMapObject::ChangeMapObject(Document *document,
                                 MapObject *mapObject,
                                 MapObject::Property property,
                                 const QVariant &value)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Object"))
    , mDocument(document)
    , mMapObject(mapObject)
    , mProperty(property)
    , mValue(value)
    , mOldChangeState(mapObject->propertyChanged(property))
    , mNewChangeState(true)
{
    switch (property) {
    case MapObject::VisibleProperty:
        if (value.toBool())
            setText(QCoreApplication::translate("Undo Commands", "Show Object"));
        else
            setText(QCoreApplication::translate("Undo Commands", "Hide Object"));
        break;
    default:
        break;
    }
}

void ChangeMapObject::swap()
{
    QVariant oldValue = mMapObject->mapObjectProperty(mProperty);
    mMapObject->setMapObjectProperty(mProperty, mValue);
    std::swap(mValue, oldValue);

    mMapObject->setPropertyChanged(mProperty, mNewChangeState);
    std::swap(mOldChangeState, mNewChangeState);

    emit mDocument->changed(MapObjectsChangeEvent(mMapObject, mProperty));
}


ChangeMapObjectCells::ChangeMapObjectCells(Document *document,
                                           const QVector<MapObjectCell> &changes,
                                           QUndoCommand *parent)
    : QUndoCommand(parent)
    , mDocument(document)
    , mChanges(changes)
{
}

static QList<MapObject*> objectList(const QVector<MapObjectCell> &changes)
{
    QList<MapObject*> result;
    result.reserve(changes.size());

    for (const MapObjectCell &change : changes)
        result.append(change.object);

    return result;
}

void ChangeMapObjectCells::swap()
{
    for (int i = 0; i < mChanges.size(); ++i) {
        MapObjectCell &change = mChanges[i];

        auto cell = change.object->cell();
        change.object->setCell(change.cell);
        change.cell = cell;

        auto changed = change.object->propertyChanged(MapObject::CellProperty);
        change.object->setPropertyChanged(MapObject::CellProperty, change.propertyChanged);
        change.propertyChanged = changed;
    }

    emit mDocument->changed(MapObjectsChangeEvent(objectList(mChanges), MapObject::CellProperty));
}


ChangeMapObjectsTile::ChangeMapObjectsTile(Document *document,
                                           const QList<MapObject *> &mapObjects,
                                           Tile *tile)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change %n Object/s Tile",
                                               nullptr, mapObjects.size()))
    , mDocument(document)
    , mMapObjects(mapObjects)
    , mTile(tile)
{
    for (MapObject *object : qAsConst(mMapObjects)) {
        Cell cell = object->cell();
        mOldCells.append(cell);
        Tile *tile = cell.tile();
        // Update the size if the object's tile is valid and the sizes match
        mUpdateSize.append(tile && object->size() == tile->size());

        mOldChangeStates.append(object->propertyChanged(MapObject::CellProperty));
    }
}

static void setObjectCell(MapObject *object,
                          const Cell &cell,
                          const bool updateSize)
{
    object->setCell(cell);

    if (updateSize)
        object->setSize(cell.tile()->size());
}

void ChangeMapObjectsTile::undo()
{
    restoreTiles();
    QUndoCommand::undo(); // undo child commands
}

void ChangeMapObjectsTile::redo()
{
    QUndoCommand::redo(); // redo child commands
    changeTiles();
}

void ChangeMapObjectsTile::restoreTiles()
{
    for (int i = 0; i < mMapObjects.size(); ++i) {
        setObjectCell(mMapObjects[i], mOldCells[i], mUpdateSize[i]);
        mMapObjects[i]->setPropertyChanged(MapObject::CellProperty, mOldChangeStates[i]);
    }

    emit mDocument->changed(MapObjectsChangeEvent(mMapObjects, MapObject::CellProperty));
}

void ChangeMapObjectsTile::changeTiles()
{
    for (int i = 0; i < mMapObjects.size(); ++i) {
        Cell cell = mMapObjects[i]->cell();
        cell.setTile(mTile);
        setObjectCell(mMapObjects[i], cell, mUpdateSize[i]);
        mMapObjects[i]->setPropertyChanged(MapObject::CellProperty);
    }

    emit mDocument->changed(MapObjectsChangeEvent(mMapObjects, MapObject::CellProperty));
}

DetachObjects::DetachObjects(Document *document,
                             const QList<MapObject *> &mapObjects,
                             QUndoCommand *parent)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Detach %n Template Instance(s)",
                                               nullptr, mapObjects.size()), parent)
    , mDocument(document)
    , mMapObjects(mapObjects)
{
    for (const MapObject *object : mapObjects) {
        mObjectTemplates.append(object->objectTemplate());
        mProperties.append(object->properties());
    }
}

void DetachObjects::redo()
{
    QUndoCommand::redo(); // redo child commands

    for (MapObject *object : qAsConst(mMapObjects))
        object->detachFromTemplate();

    emit mDocument->changed(MapObjectsChangeEvent(mMapObjects, MapObject::TemplateProperty));
}

void DetachObjects::undo()
{
    for (int i = 0; i < mMapObjects.size(); ++i) {
        MapObject *object = mMapObjects.at(i);
        object->setObjectTemplate(mObjectTemplates.at(i));
        object->setProperties(mProperties.at(i));
        object->syncWithTemplate();
    }

    QUndoCommand::undo(); // undo child commands

    emit mDocument->changed(MapObjectsChangeEvent(mMapObjects, MapObject::TemplateProperty));
}

ResetInstances::ResetInstances(Document *document,
                               const QList<MapObject *> &mapObjects,
                               QUndoCommand *parent)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Reset %n Instances",
                                               nullptr, mapObjects.size()), parent)
    , mDocument(document)
    , mMapObjects(mapObjects)
{
    for (const MapObject *object : mapObjects)
        mOldMapObjects.append(object->clone());
}

ResetInstances::~ResetInstances()
{
    qDeleteAll(mOldMapObjects);
}

void ResetInstances::redo()
{
    MapObject::ChangedProperties affectedProperties = MapObject::CustomProperties;

    for (auto object : mMapObjects) {
        // Template instances initially don't hold any custom properties
        object->clearProperties();

        affectedProperties |= object->changedProperties();

        // Reset built-in properties
        object->setChangedProperties(MapObject::ChangedProperties());
        object->syncWithTemplate();
    }

    emit mDocument->changed(MapObjectsChangeEvent(mMapObjects, affectedProperties));

    // This signal forces updating custom properties in the properties dock
//    emit mMapDocument->selectedObjectsChanged();
}

void ResetInstances::undo()
{
    MapObject::ChangedProperties affectedProperties = MapObject::CustomProperties;

    for (int i = 0; i < mMapObjects.size(); ++i) {
        mMapObjects.at(i)->copyPropertiesFrom(mOldMapObjects.at(i));
        affectedProperties |= mOldMapObjects.at(i)->changedProperties();
    }

    emit mDocument->changed(MapObjectsChangeEvent(mMapObjects, affectedProperties));
}


ReplaceObjectsWithTemplate::ReplaceObjectsWithTemplate(Document *document,
                                                       const QList<MapObject *> &mapObjects,
                                                       ObjectTemplate *objectTemplate,
                                                       QUndoCommand *parent)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Replace %n Object(s) With Template",
                                               nullptr, mapObjects.size()), parent)
    , mDocument(document)
    , mMapObjects(mapObjects)
    , mObjectTemplate(objectTemplate)
{
    for (const MapObject *object : mapObjects)
        mOldMapObjects.append(object->clone());
}

ReplaceObjectsWithTemplate::~ReplaceObjectsWithTemplate()
{
    qDeleteAll(mOldMapObjects);
}

void ReplaceObjectsWithTemplate::redo()
{
    for (auto object : mMapObjects) {
        object->clearProperties();
        object->setChangedProperties(MapObject::ChangedProperties());
        object->setObjectTemplate(mObjectTemplate);
        object->syncWithTemplate();
    }

    emit mDocument->changed(MapObjectsChangeEvent(mMapObjects, MapObject::AllProperties));
}

void ReplaceObjectsWithTemplate::undo()
{
    for (int i = 0; i < mMapObjects.size(); ++i)
        mMapObjects.at(i)->copyPropertiesFrom(mOldMapObjects.at(i));

    emit mDocument->changed(MapObjectsChangeEvent(mMapObjects, MapObject::AllProperties));
}
