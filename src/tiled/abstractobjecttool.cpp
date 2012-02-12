/*
 * abstractobjecttool.cpp
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "abstractobjecttool.h"

#include "addremovemapobject.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "movemapobjecttogroup.h"
#include "objectgroup.h"
#include "objectpropertiesdialog.h"
#include "utils.h"

#include <QMenu>
#include <QUndoStack>

#include <cmath>

using namespace Tiled;
using namespace Tiled::Internal;

AbstractObjectTool::AbstractObjectTool(const QString &name,
                                       const QIcon &icon,
                                       const QKeySequence &shortcut,
                                       QObject *parent)
    : AbstractTool(name, icon, shortcut, parent)
    , mMapScene(0)
{
}

void AbstractObjectTool::activate(MapScene *scene)
{
    mMapScene = scene;
}

void AbstractObjectTool::deactivate(MapScene *)
{
    mMapScene = 0;
}

void AbstractObjectTool::mouseLeft()
{
    setStatusInfo(QString());
}

void AbstractObjectTool::mouseMoved(const QPointF &pos,
                                    Qt::KeyboardModifiers)
{
    const QPointF tilePosF = mapDocument()->renderer()->pixelToTileCoords(pos);
    const int x = (int) std::floor(tilePosF.x());
    const int y = (int) std::floor(tilePosF.y());
    setStatusInfo(QString(QLatin1String("%1, %2")).arg(x).arg(y));
}

void AbstractObjectTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        showContextMenu(topMostObjectItemAt(event->scenePos()),
                        event->screenPos(), event->widget());
    }
}

void AbstractObjectTool::updateEnabledState()
{
    setEnabled(currentObjectGroup() != 0);
}

ObjectGroup *AbstractObjectTool::currentObjectGroup() const
{
    if (!mapDocument())
        return 0;

    return dynamic_cast<ObjectGroup*>(mapDocument()->currentLayer());
}

MapObjectItem *AbstractObjectTool::topMostObjectItemAt(QPointF pos) const
{
    foreach (QGraphicsItem *item, mMapScene->items(pos)) {
        if (MapObjectItem *objectItem = dynamic_cast<MapObjectItem*>(item))
            return objectItem;
    }
    return 0;
}

/**
 * Shows the context menu for map objects. The menu allows you to duplicate and
 * remove the map objects, or to edit their properties.
 */
void AbstractObjectTool::showContextMenu(MapObjectItem *clickedObjectItem,
                                         QPoint screenPos, QWidget *parent)
{
    QSet<MapObjectItem *> selection = mMapScene->selectedObjectItems();
    if (clickedObjectItem && !selection.contains(clickedObjectItem)) {
        selection.clear();
        selection.insert(clickedObjectItem);
        mMapScene->setSelectedObjectItems(selection);
    }
    if (selection.isEmpty())
        return;

    const QList<MapObject*> selectedObjects = mapDocument()->selectedObjects();

    QList<ObjectGroup*> objectGroups;
    foreach (Layer *layer, mapDocument()->map()->layers()) {
        if (ObjectGroup *objectGroup = layer->asObjectGroup())
            objectGroups.append(objectGroup);
    }

    QMenu menu;
    QIcon dupIcon(QLatin1String(":images/16x16/stock-duplicate-16.png"));
    QIcon delIcon(QLatin1String(":images/16x16/edit-delete.png"));
    QIcon propIcon(QLatin1String(":images/16x16/document-properties.png"));
    QString dupText = tr("Duplicate %n Object(s)", "", selectedObjects.size());
    QString dupText2 = tr("Clone Hierarchy of %n Object(s)", "", selectedObjects.size());
    QString removeText = tr("Remove %n Object(s)", "", selectedObjects.size());
    QAction *cloneHierarchyAction = selectedObjects.size() > 1 ? NULL : menu.addAction(dupIcon, dupText2);
    QAction *dupAction = menu.addAction(dupIcon, dupText);
    QAction *removeAction = menu.addAction(delIcon, removeText);

    typedef QMap<QAction*, ObjectGroup*> MoveToLayerActionMap;
    MoveToLayerActionMap moveToLayerActions;

    if (objectGroups.size() > 1) {
        menu.addSeparator();
        QMenu *moveToLayerMenu = menu.addMenu(tr("Move %n Object(s) to Layer",
                                                 "", selectedObjects.size()));
        foreach (ObjectGroup *objectGroup, objectGroups) {
            QAction *action = moveToLayerMenu->addAction(objectGroup->name());
            moveToLayerActions.insert(action, objectGroup);
        }
    }

    menu.addSeparator();
    QAction *propertiesAction = menu.addAction(propIcon,
                                               tr("Object &Properties..."));
    // TODO: Implement editing of properties for multiple objects
    propertiesAction->setEnabled(selectedObjects.size() == 1);

    Utils::setThemeIcon(removeAction, "edit-delete");
    Utils::setThemeIcon(propertiesAction, "document-properties");

    QAction *selectedAction = menu.exec(screenPos);

    if (selectedAction == dupAction) {
        duplicateObjects(selectedObjects,false);
    }
    else if (selectedAction == cloneHierarchyAction) {
        duplicateObjects(selectedObjects,true);
    }
    else if (selectedAction == removeAction) {
        removeObjects(selectedObjects);
    }
    else if (selectedAction == propertiesAction) {
        MapObject *mapObject = selectedObjects.first();
        ObjectPropertiesDialog propertiesDialog(mapDocument(), mapObject,
                                                parent);
        propertiesDialog.exec();
    }

    MoveToLayerActionMap::const_iterator i =
            moveToLayerActions.find(selectedAction);

    if (i != moveToLayerActions.end()) {
        ObjectGroup *objectGroup = i.value();
        moveObjectsToGroup(selectedObjects, objectGroup);
    }
}

void AbstractObjectTool::cloneHierarchy(MapObject* pParent, QMap<quint32,quint32>& idMap, QList<MapObject*>& clones)
{
    QUndoStack *undoStack = mapDocument()->undoStack();

     //Clone all linked MapObjects
    Properties::const_iterator it = pParent->properties().constBegin();
    Properties::const_iterator it_end = pParent->properties().constEnd();
    for (; it != it_end; ++it) {
        const Property* value = &it.value();
        if(value->Type() == Property::PropertyType_Link)
        {
            //Get the pointer to the linked MapObject
            MapObject* pChild = mMapScene->mapDocument()->map()->getMapObjectFromQMap(value->toUInt());

            //Only do something if we found the child (could be an invalid link)
            if(pChild != NULL)
            {
                //Check to see if the child is already in the id map
                const quint32 cloneID = idMap.value(pChild->uniqueID());

                //If the ID was found, then we've already spawned the clone so just
                //link to the ID of the clone
                if(cloneID != 0)
                {
                    //Update the property to link to the new UniqueID
                    //TODO: sometimes passing in a number and not a QString is better ;)
                    pParent->setProperty(it.key(),Property::FromQString(Property::PropertyType_Link,QString::number(cloneID)));
                }
                //If it was not in the map, then we need to clone and clone the hierarchy
                else
                {
                    //Clone the child
                    MapObject *childClone = pChild->clone();

                    //Give the child a new UniqueID and add it to the QMap
                    const quint32 uniqueID = mMapScene->mapDocument()->map()->createUniqueID();
                    childClone->setUniqueID(uniqueID);
                    mMapScene->mapDocument()->map()->addToQMap(childClone);

                    //Update the property to link to the new UniqueID
                    //TODO: sometimes passing in a number and not a QString is better ;)
                    pParent->setProperty(it.key(),Property::FromQString(Property::PropertyType_Link,QString::number(uniqueID)));

                    //Add to undo
                    clones.append(childClone);

                    //After AddMapObject, the clone with have an ObjectGroup
                    undoStack->push(new AddMapObject(mapDocument(),
                                                     pParent->objectGroup(),
                                                     childClone));

                    //Add to id map (maps old ID to new ID)
                    idMap.insert(pChild->uniqueID(),uniqueID);

                    //Recursively clone the hierarchy
                    cloneHierarchy(childClone, idMap, clones);
                }
            }
        }
    }
}

void AbstractObjectTool::duplicateObjects(const QList<MapObject *> &objects, bool shouldCloneHierarchy)
{
    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Duplicate %n Object(s)", "", objects.size()));

    QList<MapObject*> clones;
    foreach (const MapObject *mapObject, objects) {
        MapObject *clonedObject = mapObject->clone();

        const quint32 uniqueID = mMapScene->mapDocument()->map()->createUniqueID();
        clonedObject->setUniqueID(uniqueID);
        mMapScene->mapDocument()->map()->addToQMap(clonedObject);

        clones.append(clonedObject);

        //After AddMapObject, the clone with have an ObjectGroup
        undoStack->push(new AddMapObject(mapDocument(),
                                         mapObject->objectGroup(),
                                         clonedObject));

        //Now clone the links
        if(shouldCloneHierarchy)
        {
            QMap<quint32,quint32> idMap;

            //Add parent to id map (maps old ID to new ID)
            idMap.insert(clonedObject->uniqueID(),uniqueID);

            //Clone all linked MapObjects
            cloneHierarchy(clonedObject,idMap,clones);
        }
    }

    undoStack->endMacro();
    mapDocument()->setSelectedObjects(clones);
}

void AbstractObjectTool::removeObjects(const QList<MapObject *> &objects)
{
    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Remove %n Object(s)", "", objects.size()));
    foreach (MapObject *mapObject, objects)
        undoStack->push(new RemoveMapObject(mapDocument(), mapObject));
    undoStack->endMacro();
}

void AbstractObjectTool::moveObjectsToGroup(const QList<MapObject *> &objects,
                                            ObjectGroup *objectGroup)
{
    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Move %n Object(s) to Layer", "",
                             objects.size()));
    foreach (MapObject *mapObject, objects) {
        if (mapObject->objectGroup() == objectGroup)
            continue;
        undoStack->push(new MoveMapObjectToGroup(mapDocument(),
                                                 mapObject,
                                                 objectGroup));
    }
    undoStack->endMacro();
}
