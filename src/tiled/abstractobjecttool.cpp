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

#include "map.h"
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "objectgroup.h"
#include "raiselowerhelper.h"
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
                        event->screenPos());
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

void AbstractObjectTool::flipHorizontally()
{
    mapDocument()->flipSelectedObjects(FlipHorizontally);
}

void AbstractObjectTool::flipVertically()
{
    mapDocument()->flipSelectedObjects(FlipVertically);
}

void AbstractObjectTool::raise()
{
    RaiseLowerHelper(mMapScene).raise();
}

void AbstractObjectTool::lower()
{
    RaiseLowerHelper(mMapScene).lower();
}

void AbstractObjectTool::raiseToTop()
{
    RaiseLowerHelper(mMapScene).raiseToTop();
}

void AbstractObjectTool::lowerToBottom()
{
    RaiseLowerHelper(mMapScene).lowerToBottom();
}

/**
 * Shows the context menu for map objects. The menu allows you to duplicate and
 * remove the map objects, or to edit their properties.
 */
void AbstractObjectTool::showContextMenu(MapObjectItem *clickedObjectItem,
                                         QPoint screenPos)
{
    QSet<MapObjectItem *> selection = mMapScene->selectedObjectItems();
    if (clickedObjectItem && !selection.contains(clickedObjectItem)) {
        selection.clear();
        selection.insert(clickedObjectItem);
        mMapScene->setSelectedObjectItems(selection);
    }
    if (selection.isEmpty())
        return;

    const QList<MapObject*> &selectedObjects = mapDocument()->selectedObjects();

    QList<ObjectGroup*> objectGroups;
    foreach (Layer *layer, mapDocument()->map()->layers()) {
        if (ObjectGroup *objectGroup = layer->asObjectGroup())
            objectGroups.append(objectGroup);
    }

    MapDocumentActionHandler *handler = MapDocumentActionHandler::instance();

    QMenu menu;
    menu.addAction(handler->actionDuplicateObjects());
    menu.addAction(handler->actionRemoveObjects());

    menu.addSeparator();
    QAction *horizontalAction = menu.addAction(tr("Flip Horizontally"));
    QAction *verticalAction = menu.addAction(tr("Flip Vertically"));
    connect(horizontalAction, SIGNAL(triggered()), SLOT(flipHorizontally()));
    connect(verticalAction, SIGNAL(triggered()), SLOT(flipVertically()));

    menu.addSeparator();
    QAction *raiseAction = menu.addAction(tr("Raise Object"));
    QAction *lowerAction = menu.addAction(tr("Lower Object"));
    QAction *raiseToTopAction = menu.addAction(tr("Raise Object to Top"));
    QAction *lowerToBottomAction = menu.addAction(tr("Lower Object to Bottom"));
    connect(raiseAction, SIGNAL(triggered()), SLOT(raise()));
    connect(lowerAction, SIGNAL(triggered()), SLOT(lower()));
    connect(raiseToTopAction, SIGNAL(triggered()), SLOT(raiseToTop()));
    connect(lowerToBottomAction, SIGNAL(triggered()), SLOT(lowerToBottom()));

    if (objectGroups.size() > 1) {
        menu.addSeparator();
        QMenu *moveToLayerMenu = menu.addMenu(tr("Move %n Object(s) to Layer",
                                                 "", selectedObjects.size()));
        foreach (ObjectGroup *objectGroup, objectGroups) {
            QAction *action = moveToLayerMenu->addAction(objectGroup->name());
            action->setData(QVariant::fromValue(objectGroup));
        }
    }

    QAction *action = menu.exec(screenPos);
    if (!action)
        return;

    if (ObjectGroup *objectGroup = action->data().value<ObjectGroup*>())
        handler->moveObjectsToGroup(objectGroup);
}
