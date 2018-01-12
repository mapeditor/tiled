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

using namespace Tiled;
using namespace Tiled::Internal;

ChangeMapObject::ChangeMapObject(MapDocument *mapDocument,
                                 MapObject *mapObject,
                                 MapObject::Property property,
                                 const QVariant &value)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change Object"))
    , mMapDocument(mapDocument)
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
    mMapDocument->mapObjectModel()->setObjectProperty(mMapObject, mProperty, mValue);
    std::swap(mValue, oldValue);

    mMapObject->setPropertyChanged(mProperty, mNewChangeState);
    std::swap(mOldChangeState, mNewChangeState);
}


ChangeMapObjectCells::ChangeMapObjectCells(MapDocument *mapDocument,
                                           const QVector<MapObjectCell> &changes,
                                           QUndoCommand *parent)
    : QUndoCommand(parent)
    , mMapObjectModel(mapDocument->mapObjectModel())
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
    for (MapObjectCell &change : mChanges) {
        auto cell = change.object->cell();
        change.object->setCell(change.cell);
        change.cell = cell;
    }
    emit mMapObjectModel->objectsChanged(objectList(mChanges));
}

ChangeMapObjectsTile::ChangeMapObjectsTile(MapDocument *mapDocument,
                                           const QList<MapObject *> &mapObjects,
                                           Tile *tile)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Change %n Object/s Tile",
                                               nullptr, mapObjects.size()))
    , mMapDocument(mapDocument)
    , mMapObjects(mapObjects)
    , mTile(tile)
{
    for (MapObject *object : mMapObjects) {
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

    emit mMapDocument->mapObjectModel()->objectsChanged(mMapObjects);
}

void ChangeMapObjectsTile::changeTiles()
{
    for (int i = 0; i < mMapObjects.size(); ++i) {
        Cell cell = mMapObjects[i]->cell();
        cell.setTile(mTile);
        setObjectCell(mMapObjects[i], cell, mUpdateSize[i]);
        mMapObjects[i]->setPropertyChanged(MapObject::CellProperty);
    }

    emit mMapDocument->mapObjectModel()->objectsChanged(mMapObjects);
}

DetachObjects::DetachObjects(MapDocument *mapDocument,
                             const QList<MapObject *> &mapObjects,
                             QUndoCommand *parent)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Detach %n Template Instance(s)",
                                               nullptr, mapObjects.size()), parent)
    , mMapDocument(mapDocument)
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

    for (int i = 0; i < mMapObjects.size(); ++i) {
        // Merge the instance properties into the template properties
        MapObject *object = mMapObjects.at(i);
        Properties newProperties = object->templateObject()->properties();
        newProperties.merge(object->properties());
        object->setProperties(newProperties);
        object->setObjectTemplate(nullptr);
    }

    emit mMapDocument->mapObjectModel()->objectsChanged(mMapObjects);
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

    emit mMapDocument->mapObjectModel()->objectsChanged(mMapObjects);
}

ResetInstances::ResetInstances(MapDocument *mapDocument,
                               const QList<MapObject *> &mapObjects,
                               QUndoCommand *parent)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Reset %n Instances",
                                               nullptr, mapObjects.size()), parent)
    , mMapDocument(mapDocument)
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
    for (auto object : mMapObjects) {
        // Template instances initially don't hold any custom properties
        object->clearProperties();

        // Reset built-in properties
        object->setChangedProperties(MapObject::ChangedProperties());
        object->syncWithTemplate();
    }

    emit mMapDocument->objectsChanged(mMapObjects);

    // This signal forces updating custom properties in the properties dock
    emit mMapDocument->selectedObjectsChanged();
}

void ResetInstances::undo()
{
    for (int i = 0; i < mMapObjects.size(); ++i)
        mMapObjects.at(i)->copyPropertiesFrom(mOldMapObjects.at(i));

    emit mMapDocument->objectsChanged(mMapObjects);
    emit mMapDocument->selectedObjectsChanged();
}


ReplaceObjectsWithTemplate::ReplaceObjectsWithTemplate(MapDocument *mapDocument,
                                                       const QList<MapObject *> &mapObjects,
                                                       ObjectTemplate *objectTemplate,
                                                       QUndoCommand *parent)
    : QUndoCommand(QCoreApplication::translate("Undo Commands",
                                               "Replace %n Object(s) With Template",
                                               nullptr, mapObjects.size()), parent)
    , mMapDocument(mapDocument)
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

    emit mMapDocument->objectsChanged(mMapObjects);
    emit mMapDocument->selectedObjectsChanged();
}

void ReplaceObjectsWithTemplate::undo()
{
    for (int i = 0; i < mMapObjects.size(); ++i)
        mMapObjects.at(i)->copyPropertiesFrom(mOldMapObjects.at(i));

    emit mMapDocument->objectsChanged(mMapObjects);
    emit mMapDocument->selectedObjectsChanged();
}
