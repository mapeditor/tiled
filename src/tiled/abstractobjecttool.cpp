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
    QString removeText = tr("Remove %n Object(s)", "", selectedObjects.size());
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
        duplicateObjects(selectedObjects);
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

void AbstractObjectTool::duplicateObjects(const QList<MapObject *> &objects)
{
    QUndoStack *undoStack = mapDocument()->undoStack();
    undoStack->beginMacro(tr("Duplicate %n Object(s)", "", objects.size()));

    QList<MapObject*> clones;
    foreach (const MapObject *mapObject, objects) {
        MapObject *clone = mapObject->clone();
        clones.append(clone);
        undoStack->push(new AddMapObject(mapDocument(),
                                         mapObject->objectGroup(),
                                         clone));
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
