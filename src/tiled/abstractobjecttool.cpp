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

#include "addremovetileset.h"
#include "changemapobject.h"
#include "documentmanager.h"
//#include "fileformat.h"
#include "mapdocument.h"
#include "map.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "objectgroup.h"
#include "preferences.h"
#include "raiselowerhelper.h"
#include "resizemapobject.h"
#include "templatemanager.h"
#include "tile.h"
#include "tmxmapformat.h"
#include "utils.h"

#include <QFileDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QUndoStack>

#include <QtMath>

using namespace Tiled;
using namespace Tiled::Internal;

static bool isTileObject(MapObject *mapObject)
{
    return !mapObject->cell().isEmpty();
}

static bool isTemplateInstance(MapObject *mapObject)
{
    return mapObject->isTemplateInstance();
}

static bool isResizedTileObject(MapObject *mapObject)
{
    if (const auto tile = mapObject->cell().tile())
        return mapObject->size() != tile->size();
    return false;
}

static bool isChangedTemplateInstance(MapObject *mapObject)
{
    if (const MapObject *templateObject = mapObject->templateObject()) {
        return mapObject->changedProperties() != 0 ||
               mapObject->properties() != templateObject->properties();
    }
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
    const int x = qFloor(tilePosF.x());
    const int y = qFloor(tilePosF.y());
    setStatusInfo(QString(QLatin1String("%1, %2 (%3, %4)")).arg(x).arg(y).arg(pixelPos.x()).arg(pixelPos.y()));
}

void AbstractObjectTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        showContextMenu(topMostMapObjectAt(event->scenePos()),
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

QList<MapObject*> AbstractObjectTool::mapObjectsAt(const QPointF &pos) const
{
    const QList<QGraphicsItem *> &items = mMapScene->items(pos);

    QList<MapObject*> objectList;
    for (auto item : items) {
        MapObjectItem *objectItem = qgraphicsitem_cast<MapObjectItem*>(item);
        if (objectItem && objectItem->mapObject()->objectGroup()->isUnlocked())
            objectList.append(objectItem->mapObject());
    }
    return objectList;
}

MapObject *AbstractObjectTool::topMostMapObjectAt(const QPointF &pos) const
{
    const QList<QGraphicsItem *> &items = mMapScene->items(pos);

    for (QGraphicsItem *item : items) {
        MapObjectItem *objectItem = qgraphicsitem_cast<MapObjectItem*>(item);
        if (objectItem && objectItem->mapObject()->objectGroup()->isUnlocked())
            return objectItem->mapObject();
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

static QString saveObjectTemplate(const MapObject *mapObject)
{
    FormatHelper<ObjectTemplateFormat> helper(FileFormat::ReadWrite);
    QString filter = helper.filter();
    QString selectedFilter = XmlObjectTemplateFormat().nameFilter();

    Preferences *prefs = Preferences::instance();
    QString suggestedFileName = prefs->lastPath(Preferences::ObjectTemplateFile);
    suggestedFileName += QLatin1Char('/');
    if (!mapObject->name().isEmpty())
        suggestedFileName += mapObject->name();
    else
        suggestedFileName += QCoreApplication::translate("Tiled::Internal::MainWindow", "untitled");
    suggestedFileName += QLatin1String(".tx");

    QWidget *parent = DocumentManager::instance()->widget()->window();
    QString fileName = QFileDialog::getSaveFileName(parent,
                                                    QCoreApplication::translate("Tiled::Internal::MainWindow", "Save Template"),
                                                    suggestedFileName,
                                                    filter,
                                                    &selectedFilter);

    if (fileName.isEmpty())
        return QString();

    ObjectTemplateFormat *format = helper.formatByNameFilter(selectedFilter);

    ObjectTemplate objectTemplate;
    objectTemplate.setObject(mapObject);

    if (!format->write(&objectTemplate, fileName)) {
        QMessageBox::critical(nullptr, QCoreApplication::translate("Tiled::Internal::MainWindow", "Error Saving Template"),
                              format->errorString());
        return QString();
    }

    prefs->setLastPath(Preferences::ObjectTemplateFile,
                       QFileInfo(fileName).path());

    return fileName;
}

void AbstractObjectTool::saveSelectedObject()
{
    auto object = mapDocument()->selectedObjects().first();
    QString fileName = saveObjectTemplate(object);
    if (fileName.isEmpty())
        return;

    // Convert the saved object into an instance
    if (ObjectTemplate *objectTemplate = TemplateManager::instance()->loadObjectTemplate(fileName))
        mapDocument()->undoStack()->push(new ReplaceObjectsWithTemplate(mapDocument(), { object }, objectTemplate));
}

void AbstractObjectTool::detachSelectedObjects()
{
    MapDocument *currentMapDocument = mapDocument();
    QList<MapObject *> templateInstances;

    /**
     * Stores the unique tilesets used by the templates
     * to avoid creating multiple undo commands for the same tileset
     */
    QSet<SharedTileset> sharedTilesets;

    for (MapObject *object : mapDocument()->selectedObjects()) {
        if (object->templateObject()) {
            templateInstances.append(object);

            if (Tile *tile = object->cell().tile())
                sharedTilesets.insert(tile->tileset()->sharedPointer());
        }
    }

    auto changeMapObjectCommand = new DetachObjects(currentMapDocument, templateInstances);

    // Add any missing tileset used by the templates to the map map before detaching
    for (SharedTileset sharedTileset : sharedTilesets) {
        if (!currentMapDocument->map()->tilesets().contains(sharedTileset))
            new AddTileset(currentMapDocument, sharedTileset, changeMapObjectCommand);
    }

    currentMapDocument->undoStack()->push(changeMapObjectCommand);
}

void AbstractObjectTool::replaceObjectsWithTemplate()
{
    mapDocument()->undoStack()->push(new ReplaceObjectsWithTemplate(mapDocument(),
                                                                    mapDocument()->selectedObjects(),
                                                                    objectTemplate()));
}

void AbstractObjectTool::resetInstances()
{
    QList<MapObject *> templateInstances;

    for (MapObject *object : mapDocument()->selectedObjects()) {
        if (object->templateObject())
            templateInstances.append(object);
    }

    mapDocument()->undoStack()->push(new ResetInstances(mapDocument(), templateInstances));
}

void AbstractObjectTool::changeTile()
{
    QList<MapObject*> tileObjects;

    MapDocument *currentMapDocument = mapDocument();

    for (auto object : currentMapDocument->selectedObjects())
        if (object->isTileObject())
            tileObjects.append(object);

    auto changeMapObjectCommand = new ChangeMapObjectsTile(currentMapDocument, tileObjects, tile());

    // Make sure the tileset is part of the document
    SharedTileset sharedTileset = tile()->tileset()->sharedPointer();
    if (!currentMapDocument->map()->tilesets().contains(sharedTileset))
        new AddTileset(currentMapDocument, sharedTileset, changeMapObjectCommand);

    currentMapDocument->undoStack()->push(changeMapObjectCommand);
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
void AbstractObjectTool::showContextMenu(MapObject *clickedObject,
                                         QPoint screenPos)
{
    const QList<MapObject*> &selectedObjects = mapDocument()->selectedObjects();

    if (clickedObject && !selectedObjects.contains(clickedObject))
        mapDocument()->setSelectedObjects({ clickedObject });

    if (selectedObjects.isEmpty())
        return;

    QMenu menu;
    QAction *duplicateAction = menu.addAction(tr("Duplicate %n Object(s)", "", selectedObjects.size()),
                                              this, SLOT(duplicateObjects()));
    QAction *removeAction = menu.addAction(tr("Remove %n Object(s)", "", selectedObjects.size()),
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

        auto changeTileAction = menu.addAction(tr("Replace Tile"), this, SLOT(changeTile()));
        changeTileAction->setEnabled(tile() && (!selectedObjects.first()->isTemplateBase() ||
                                                tile()->tileset()->isExternal()));
    }

    // Create action for replacing an object with a template
    auto selectedTemplate = objectTemplate();
    auto replaceTemplateAction = menu.addAction(tr("Replace With Template"), this, SLOT(replaceObjectsWithTemplate()));

    if (selectedTemplate) {
        QString name = QFileInfo(selectedTemplate->fileName()).fileName();
        replaceTemplateAction->setText(tr("Replace With Template \"%1\"").arg(name));
    } else {
        replaceTemplateAction->setEnabled(false);
    }

    if (selectedObjects.size() == 1) {
        MapObject *currentObject = selectedObjects.first();

        if (!(currentObject->isTemplateBase() || currentObject->isTemplateInstance())) {
            const Cell cell = selectedObjects.first()->cell();
            // Saving objects with embedded tilesets is disabled
            if (cell.isEmpty() || cell.tileset()->isExternal())
                menu.addAction(tr("Save As Template"), this, SLOT(saveSelectedObject()));
        }

        if (currentObject->isTemplateBase()) { // Hide this operations for template base
            duplicateAction->setVisible(false);
            removeAction->setVisible(false);
            replaceTemplateAction->setVisible(false);
        }
    }

    bool anyTemplateInstanceSelected = std::any_of(selectedObjects.begin(),
                                                   selectedObjects.end(),
                                                   isTemplateInstance);

    if (anyTemplateInstanceSelected) {
        menu.addAction(tr("Detach"), this, SLOT(detachSelectedObjects()));

        auto resetToTemplateAction = menu.addAction(tr("Reset Template Instance(s)"), this, SLOT(resetInstances()));
        resetToTemplateAction->setEnabled(std::any_of(selectedObjects.begin(),
                                                      selectedObjects.end(),
                                                      isChangedTemplateInstance));
    }

    menu.addSeparator();
    menu.addAction(tr("Flip Horizontally"), this, SLOT(flipHorizontally()), QKeySequence(tr("X")));
    menu.addAction(tr("Flip Vertically"), this, SLOT(flipVertically()), QKeySequence(tr("Y")));

    ObjectGroup *objectGroup = RaiseLowerHelper::sameObjectGroup(selectedObjects);
    if (objectGroup && objectGroup->drawOrder() == ObjectGroup::IndexOrder) {
        menu.addSeparator();
        menu.addAction(tr("Raise Object"), this, SLOT(raise()), QKeySequence(tr("PgUp")));
        menu.addAction(tr("Lower Object"), this, SLOT(lower()), QKeySequence(tr("PgDown")));
        menu.addAction(tr("Raise Object to Top"), this, SLOT(raiseToTop()), QKeySequence(tr("Home")));
        menu.addAction(tr("Lower Object to Bottom"), this, SLOT(lowerToBottom()), QKeySequence(tr("End")));
    }

    const QList<ObjectGroup*> objectGroups = mapDocument()->map()->objectGroups();
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

    if (ObjectGroup *objectGroup = action->data().value<ObjectGroup*>())
        mapDocument()->moveObjectsToGroup(selectedObjects, objectGroup);
}
