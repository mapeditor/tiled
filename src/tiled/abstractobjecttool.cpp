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
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "newtemplatedialog.h"
#include "objectgroup.h"
#include "objecttemplatemodel.h"
#include "raiselowerhelper.h"
#include "resizemapobject.h"
#include "tile.h"
#include "utils.h"

#include <QKeyEvent>
#include <QMenu>
#include <QUndoStack>

#include <cmath>

using namespace Tiled;
using namespace Tiled::Internal;

static bool isTileObject(MapObject *mapObject)
{
    return !mapObject->cell().isEmpty();
}

static bool isResizedTileObject(MapObject *mapObject)
{
    if (const auto tile = mapObject->cell().tile())
        return mapObject->size() != tile->size();
    return false;
}


AbstractObjectTool::AbstractObjectTool(const QString &name,
                                       const QIcon &icon,
                                       const QKeySequence &shortcut,
                                       QObject *parent)
    : AbstractTool(name, icon, shortcut, parent)
    , mMapScene(nullptr)
{
}

void AbstractObjectTool::activate(MapScene *scene)
{
    mMapScene = scene;
}

void AbstractObjectTool::deactivate(MapScene *)
{
    mMapScene = nullptr;
}

void AbstractObjectTool::keyPressed(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_PageUp:    raise(); return;
    case Qt::Key_PageDown:  lower(); return;
    case Qt::Key_Home:      raiseToTop(); return;
    case Qt::Key_End:       lowerToBottom(); return;
    case Qt::Key_D:
        if (event->modifiers() & Qt::ControlModifier) {
            duplicateObjects();
            return;
        }
        break;
    }

    event->ignore();
}

void AbstractObjectTool::mouseLeft()
{
    setStatusInfo(QString());
}

void AbstractObjectTool::mouseMoved(const QPointF &pos,
                                    Qt::KeyboardModifiers)
{
    // Take into account the offset of the current layer
    QPointF offsetPos = pos;
    if (Layer *layer = currentLayer())
        offsetPos -= layer->totalOffset();

    const QPoint pixelPos = offsetPos.toPoint();

    const QPointF tilePosF = mapDocument()->renderer()->screenToTileCoords(offsetPos);
    const int x = (int) std::floor(tilePosF.x());
    const int y = (int) std::floor(tilePosF.y());
    setStatusInfo(QString(QLatin1String("%1, %2 (%3, %4)")).arg(x).arg(y).arg(pixelPos.x()).arg(pixelPos.y()));
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
    setEnabled(currentObjectGroup() != nullptr);
}

ObjectGroup *AbstractObjectTool::currentObjectGroup() const
{
    if (!mapDocument())
        return nullptr;

    return dynamic_cast<ObjectGroup*>(mapDocument()->currentLayer());
}

QList<MapObjectItem*> AbstractObjectTool::objectItemsAt(QPointF pos) const
{
    const QList<QGraphicsItem *> &items = mMapScene->items(pos);

    QList<MapObjectItem*> objectList;
    for (auto item : items) {
        if (MapObjectItem *objectItem = qgraphicsitem_cast<MapObjectItem*>(item))
            objectList.append(objectItem);
    }
    return objectList;
}

MapObjectItem *AbstractObjectTool::topMostObjectItemAt(QPointF pos) const
{
    const QList<QGraphicsItem *> &items = mMapScene->items(pos);

    for (QGraphicsItem *item : items) {
        if (MapObjectItem *objectItem = qgraphicsitem_cast<MapObjectItem*>(item))
            return objectItem;
    }
    return nullptr;
}

void AbstractObjectTool::duplicateObjects()
{
    mapDocument()->duplicateObjects(mapDocument()->selectedObjects());
}

void AbstractObjectTool::removeObjects()
{
    mapDocument()->removeObjects(mapDocument()->selectedObjects());
}

void AbstractObjectTool::resetTileSize()
{
    QList<QUndoCommand*> commands;

    for (auto mapObject : mapDocument()->selectedObjects()) {
        if (!isResizedTileObject(mapObject))
            continue;

        commands << new ResizeMapObject(mapDocument(),
                                        mapObject,
                                        mapObject->cell().tile()->size(),
                                        mapObject->size());
    }

    if (!commands.isEmpty()) {
        QUndoStack *undoStack = mapDocument()->undoStack();
        undoStack->beginMacro(tr("Reset Tile Size"));
        for (auto command : commands)
            undoStack->push(command);
        undoStack->endMacro();
    }
}

void AbstractObjectTool::saveSelectedObject()
{
    QList<QString> names;
    auto model = ObjectTemplateModel::instance();

    for (auto *doc : model->templateDocuments())
        names.append(doc->templateGroup()->name());

    QString name;
    int groupIndex;

    NewTemplateDialog newTemplateDialog(names);
    newTemplateDialog.createTemplate(name, groupIndex);

    if (!name.isEmpty())
        mapDocument()->saveSelectedObject(name, groupIndex);
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
    const QList<ObjectGroup*> objectGroups = mapDocument()->map()->objectGroups();

    QMenu menu;
    QAction *duplicateAction = menu.addAction(tr("Duplicate %n Object(s)", "", selection.size()),
                                              this, SLOT(duplicateObjects()));
    QAction *removeAction = menu.addAction(tr("Remove %n Object(s)", "", selection.size()),
                                           this, SLOT(removeObjects()));

    duplicateAction->setIcon(QIcon(QLatin1String(":/images/16x16/stock-duplicate-16.png")));
    removeAction->setIcon(QIcon(QLatin1String(":/images/16x16/edit-delete.png")));

    bool anyTileObjectSelected = std::any_of(selectedObjects.begin(),
                                             selectedObjects.end(),
                                             isTileObject);

    if (anyTileObjectSelected) {
        auto resetTileSizeAction = menu.addAction(tr("Reset Tile Size"), this, SLOT(resetTileSize()));
        resetTileSizeAction->setEnabled(std::any_of(selectedObjects.begin(),
                                                    selectedObjects.end(),
                                                    isResizedTileObject));
    }

    if (selectedObjects.size() == 1) {
        // Saving objects with embedded tilesets is disabled
        auto cell = selectedObjects.first()->cell();
        if (cell.isEmpty() || cell.tileset()->isExternal())
            menu.addAction(tr("Save As Template"), this, SLOT(saveSelectedObject()));
    }

    menu.addSeparator();
    menu.addAction(tr("Flip Horizontally"), this, SLOT(flipHorizontally()), QKeySequence(tr("X")));
    menu.addAction(tr("Flip Vertically"), this, SLOT(flipVertically()), QKeySequence(tr("Y")));

    ObjectGroup *objectGroup = RaiseLowerHelper::sameObjectGroup(selection);
    if (objectGroup && objectGroup->drawOrder() == ObjectGroup::IndexOrder) {
        menu.addSeparator();
        menu.addAction(tr("Raise Object"), this, SLOT(raise()), QKeySequence(tr("PgUp")));
        menu.addAction(tr("Lower Object"), this, SLOT(lower()), QKeySequence(tr("PgDown")));
        menu.addAction(tr("Raise Object to Top"), this, SLOT(raiseToTop()), QKeySequence(tr("Home")));
        menu.addAction(tr("Lower Object to Bottom"), this, SLOT(lowerToBottom()), QKeySequence(tr("End")));
    }

    if (objectGroups.size() > 1) {
        menu.addSeparator();
        QMenu *moveToLayerMenu = menu.addMenu(tr("Move %n Object(s) to Layer",
                                                 "", selectedObjects.size()));
        for (ObjectGroup *objectGroup : objectGroups) {
            QAction *action = moveToLayerMenu->addAction(objectGroup->name());
            action->setData(QVariant::fromValue(objectGroup));
        }
    }

    menu.addSeparator();
    QIcon propIcon(QLatin1String(":images/16x16/document-properties.png"));
    QAction *propertiesAction = menu.addAction(propIcon,
                                               tr("Object &Properties..."));

    Utils::setThemeIcon(removeAction, "edit-delete");
    Utils::setThemeIcon(propertiesAction, "document-properties");

    QAction *action = menu.exec(screenPos);
    if (!action)
        return;

    if (action == propertiesAction) {
        MapObject *mapObject = selectedObjects.first();
        mapDocument()->setCurrentObject(mapObject);
        emit mapDocument()->editCurrentObject();
        return;
    }

    if (ObjectGroup *objectGroup = action->data().value<ObjectGroup*>()) {
        mapDocument()->moveObjectsToGroup(mapDocument()->selectedObjects(),
                                          objectGroup);
    }
}
