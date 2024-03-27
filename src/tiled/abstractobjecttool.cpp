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

#include "actionmanager.h"
#include "addremovetileset.h"
#include "changemapobject.h"
#include "changepolygon.h"
#include "changetileobjectgroup.h"
#include "documentmanager.h"
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"
#include "map.h"
#include "mapobject.h"
#include "mapobjectitem.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "objectgroup.h"
#include "raiselowerhelper.h"
#include "session.h"
#include "templatemanager.h"
#include "tile.h"
#include "tmxmapformat.h"
#include "utils.h"

#include <QFileDialog>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QToolBar>
#include <QUndoStack>

#include <QtMath>

using namespace Tiled;

static bool isTileObject(const MapObject *mapObject)
{
    return !mapObject->cell().isEmpty();
}

static bool isRectangleObject(const MapObject *mapObject)
{
    return mapObject->shape() == MapObject::Rectangle && !isTileObject(mapObject);
}

static bool isTemplateInstance(const MapObject *mapObject)
{
    return mapObject->isTemplateInstance();
}

static bool isResizedTileObject(const MapObject *mapObject)
{
    if (const auto tile = mapObject->cell().tile())
        return mapObject->size() != tile->size();
    return false;
}

static bool isChangedTemplateInstance(const MapObject *mapObject)
{
    if (const MapObject *templateObject = mapObject->templateObject()) {
        return mapObject->changedProperties() != 0 ||
               mapObject->properties() != templateObject->properties();
    }
    return false;
}

Preference<AbstractObjectTool::SelectionBehavior> AbstractObjectTool::ourSelectionBehavior {
    "AbstractObjectTool/SelectionBehavior", AbstractObjectTool::AllLayers
};

AbstractObjectTool::AbstractObjectTool(Id id,
                                       const QString &name,
                                       const QIcon &icon,
                                       const QKeySequence &shortcut,
                                       QObject *parent)
    : AbstractTool(id, name, icon, shortcut, parent)
{
    setTargetLayerType(Layer::ObjectGroupType);

    QIcon flipHorizontalIcon(QLatin1String(":images/24/flip-horizontal.png"));
    QIcon flipVerticalIcon(QLatin1String(":images/24/flip-vertical.png"));
    QIcon rotateLeftIcon(QLatin1String(":images/24/rotate-left.png"));
    QIcon rotateRightIcon(QLatin1String(":images/24/rotate-right.png"));

    flipHorizontalIcon.addFile(QLatin1String(":images/32/flip-horizontal.png"));
    flipVerticalIcon.addFile(QLatin1String(":images/32/flip-vertical.png"));
    rotateLeftIcon.addFile(QLatin1String(":images/32/rotate-left.png"));
    rotateRightIcon.addFile(QLatin1String(":images/32/rotate-right.png"));

    mFlipHorizontal = new QAction(this);
    mFlipHorizontal->setIcon(flipHorizontalIcon);
    mFlipHorizontal->setShortcut(Qt::Key_X);

    mFlipVertical = new QAction(this);
    mFlipVertical->setIcon(flipVerticalIcon);
    mFlipVertical->setShortcut(Qt::Key_Y);

    mRotateLeft = new QAction(this);
    mRotateLeft->setIcon(rotateLeftIcon);
    mRotateLeft->setShortcut(Qt::SHIFT + Qt::Key_Z);

    mRotateRight = new QAction(this);
    mRotateRight->setIcon(rotateRightIcon);
    mRotateRight->setShortcut(Qt::Key_Z);

    ActionManager::registerAction(mFlipHorizontal, "FlipHorizontal");
    ActionManager::registerAction(mFlipVertical, "FlipVertical");
    ActionManager::registerAction(mRotateLeft, "RotateLeft");
    ActionManager::registerAction(mRotateRight, "RotateRight");

    connect(mFlipHorizontal, &QAction::triggered, this, &AbstractObjectTool::flipHorizontally);
    connect(mFlipVertical, &QAction::triggered, this, &AbstractObjectTool::flipVertically);
    connect(mRotateLeft, &QAction::triggered, this, &AbstractObjectTool::rotateLeft);
    connect(mRotateRight, &QAction::triggered, this, &AbstractObjectTool::rotateRight);

    setActionsEnabled(false);

    AbstractObjectTool::languageChanged();
}

void AbstractObjectTool::activate(MapScene *scene)
{
    AbstractTool::activate(scene);
    setActionsEnabled(true);
}

void AbstractObjectTool::deactivate(MapScene *scene)
{
    setActionsEnabled(false);
    AbstractTool::deactivate(scene);
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
        offsetPos -= mapScene()->absolutePositionForLayer(*layer);

    const QPoint pixelPos = offsetPos.toPoint();

    const QPointF tilePosF = mapDocument()->renderer()->screenToTileCoords(offsetPos);
    const int x = qFloor(tilePosF.x());
    const int y = qFloor(tilePosF.y());
    setStatusInfo(QStringLiteral("%1, %2 (%3, %4)").arg(x).arg(y).arg(pixelPos.x()).arg(pixelPos.y()));
}

void AbstractObjectTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        showContextMenu(topMostMapObjectAt(event->scenePos()),
                        event->screenPos());
    }
}

void AbstractObjectTool::languageChanged()
{
    mFlipHorizontal->setText(tr("Flip Horizontally"));
    mFlipVertical->setText(tr("Flip Vertically"));
    mRotateLeft->setText(QCoreApplication::translate("Tiled::StampActions", "Rotate Left"));
    mRotateRight->setText(QCoreApplication::translate("Tiled::StampActions", "Rotate Right"));
}

void AbstractObjectTool::populateToolBar(QToolBar *toolBar)
{
    toolBar->addAction(mFlipHorizontal);
    toolBar->addAction(mFlipVertical);
    toolBar->addAction(mRotateLeft);
    toolBar->addAction(mRotateRight);
}

AbstractObjectTool::SelectionBehavior AbstractObjectTool::selectionBehavior()
{
    const SelectionBehavior behavior = ourSelectionBehavior;

    if (behavior == AllLayers && Preferences::instance()->highlightCurrentLayer())
        return PreferSelectedLayers;

    return behavior;
}

void AbstractObjectTool::filterMapObjects(QList<MapObject *> &mapObjects) const
{
    const SelectionBehavior behavior = selectionBehavior();

    if (behavior != AllLayers) {
        const auto &selectedLayers = mapDocument()->selectedLayers();

        QList<MapObject*> filteredList;

        for (MapObject *mapObject : std::as_const(mapObjects)) {
            if (std::any_of(selectedLayers.begin(), selectedLayers.end(),
                            [=] (Layer *layer) { return layer->isParentOrSelf(mapObject->objectGroup()); })) {
                filteredList.append(mapObject);
            }
        }

        if (behavior == SelectedLayers || !filteredList.isEmpty())
            mapObjects.swap(filteredList);
    }
}

ObjectGroup *AbstractObjectTool::currentObjectGroup() const
{
    if (!mapDocument())
        return nullptr;

    return dynamic_cast<ObjectGroup*>(mapDocument()->currentLayer());
}

QList<MapObject*> AbstractObjectTool::mapObjectsAt(const QPointF &pos) const
{
    const QTransform viewTransform = mapScene()->views().first()->transform();
    const QList<QGraphicsItem *> items = mapScene()->items(pos,
                                                           Qt::IntersectsItemShape,
                                                           Qt::DescendingOrder,
                                                           viewTransform);

    QList<MapObject*> objectList;

    for (auto item : items) {
        if (!item->isEnabled())
            continue;

        MapObjectItem *objectItem = qgraphicsitem_cast<MapObjectItem*>(item);
        if (objectItem && objectItem->mapObject()->objectGroup()->isUnlocked())
            objectList.append(objectItem->mapObject());
    }

    filterMapObjects(objectList);
    return objectList;
}

MapObject *AbstractObjectTool::topMostMapObjectAt(const QPointF &pos) const
{
    const QTransform viewTransform = mapScene()->views().first()->transform();
    const QList<QGraphicsItem *> items = mapScene()->items(pos,
                                                           Qt::IntersectsItemShape,
                                                           Qt::DescendingOrder,
                                                           viewTransform);
    const SelectionBehavior behavior = selectionBehavior();

    MapObject *topMost = nullptr;

    for (QGraphicsItem *item : items) {
        if (!item->isEnabled())
            continue;

        MapObjectItem *objectItem = qgraphicsitem_cast<MapObjectItem*>(item);
        if (!objectItem)
            continue;

        auto mapObject = objectItem->mapObject();
        if (!mapObject->objectGroup()->isUnlocked())
            continue;

        // Return immediately when we don't care if the layer is selected
        if (behavior == AllLayers)
            return mapObject;

        // Return this object instead of the top-most one if it is from a selected layer
        for (Layer *layer : mapDocument()->selectedLayers()) {
            if (layer->isParentOrSelf(mapObject->objectGroup()))
                return mapObject;
        }

        if (!topMost && behavior != SelectedLayers)
            topMost = mapObject;
    }

    return topMost;
}

void AbstractObjectTool::duplicateObjects()
{
    mapDocument()->duplicateObjects(mapDocument()->selectedObjects());
}

void AbstractObjectTool::removeObjects()
{
    mapDocument()->removeObjects(mapDocument()->selectedObjects());
}

/**
 * Adds the selected collision shapes for the currently selected tile - the
 * last selected tile - to all selected tiles.
 */
void AbstractObjectTool::applyCollisionsToSelectedTiles(bool replace)
{
    auto document = DocumentManager::instance()->currentDocument();
    auto tilesetDocument = qobject_cast<TilesetDocument*>(document);
    if (!tilesetDocument)
        return;

    const auto currentTile = dynamic_cast<Tile *>(tilesetDocument->currentObject());
    if (!currentTile)
        return;

    auto undoStack = tilesetDocument->undoStack();
    undoStack->beginMacro(tr("Apply Collision Shapes"));

    // The selected collision objects
    const auto &selectedObjects = mapDocument()->selectedObjects();

    // Add each collision object to each selected tile apart from the current one
    for (Tile *tile : tilesetDocument->selectedTiles()) {
        if (tile == currentTile)
            continue;

        std::unique_ptr<ObjectGroup> objectGroup;

        // Create a new group for collision objects if none exists or when replacing
        if (!tile->objectGroup() || replace)
            objectGroup = std::make_unique<ObjectGroup>();
        else
            objectGroup.reset(tile->objectGroup()->clone());

        // Copy across the selected collision shapes
        auto highestOjectId = objectGroup->highestObjectId();
        for (MapObject *object : selectedObjects) {
            MapObject *newObject = object->clone();
            newObject->setId(++highestOjectId);
            objectGroup->addObject(newObject);
        }

        undoStack->push(new ChangeTileObjectGroup(tilesetDocument,
                                                  tile,
                                                  std::move(objectGroup)));
    }

    undoStack->endMacro();
}

void AbstractObjectTool::resetTileSize()
{
    QList<QUndoCommand*> commands;

    for (auto mapObject : mapDocument()->selectedObjects()) {
        if (!isResizedTileObject(mapObject))
            continue;

        commands << new ChangeMapObject(mapDocument(),
                                        mapObject,
                                        MapObject::SizeProperty,
                                        mapObject->cell().tile()->size());
    }

    if (!commands.isEmpty()) {
        QUndoStack *undoStack = mapDocument()->undoStack();
        undoStack->beginMacro(tr("Reset Tile Size"));
        for (auto command : std::as_const(commands))
            undoStack->push(command);
        undoStack->endMacro();
    }
}

void AbstractObjectTool::convertRectanglesToPolygons()
{
    QList<QUndoCommand*> commands;

    for (auto mapObject : mapDocument()->selectedObjects()) {
        if (!isRectangleObject(mapObject))
            continue;

        const QSizeF size = mapObject->size();
        QPolygonF polygon;
        polygon.reserve(4);
        polygon.append(QPointF());
        polygon.append(QPointF(size.width(), 0.0));
        polygon.append(QPointF(size.width(), size.height()));
        polygon.append(QPointF(0.0, size.height()));

        commands << new ChangeMapObject(mapDocument(),
                                        mapObject,
                                        MapObject::ShapeProperty,
                                        MapObject::Polygon);

        commands << new ChangePolygon(mapDocument(),
                                      mapObject,
                                      polygon);
    }

    if (!commands.isEmpty()) {
        QUndoStack *undoStack = mapDocument()->undoStack();
        undoStack->beginMacro(tr("Convert to Polygon"));
        for (auto command : std::as_const(commands))
            undoStack->push(command);
        undoStack->endMacro();
    }
}

static QString saveObjectTemplate(const MapObject *mapObject)
{
    FormatHelper<ObjectTemplateFormat> helper(FileFormat::ReadWrite);
    QString filter = helper.filter();
    QString selectedFilter = XmlObjectTemplateFormat().nameFilter();

    Session &session = Session::current();
    QString suggestedFileName = session.lastPath(Session::ObjectTemplateFile);
    suggestedFileName += QLatin1Char('/');
    if (!mapObject->name().isEmpty())
        suggestedFileName += mapObject->name();
    else
        suggestedFileName += QCoreApplication::translate("Tiled::MainWindow", "untitled");
    suggestedFileName += QStringLiteral(".tx");

    QWidget *parent = DocumentManager::instance()->widget()->window();
    QString fileName = QFileDialog::getSaveFileName(parent,
                                                    QCoreApplication::translate("Tiled::MainWindow", "Save Template"),
                                                    suggestedFileName,
                                                    filter,
                                                    &selectedFilter);

    if (fileName.isEmpty())
        return QString();

    ObjectTemplateFormat *format = helper.formatByNameFilter(selectedFilter);

    ObjectTemplate objectTemplate;
    objectTemplate.setObject(mapObject);

    if (!format->write(&objectTemplate, fileName)) {
        QMessageBox::critical(nullptr, QCoreApplication::translate("Tiled::MainWindow", "Error Saving Template"),
                              format->errorString());
        return QString();
    }

    session.setLastPath(Session::ObjectTemplateFile, QFileInfo(fileName).path());

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
                sharedTilesets.insert(tile->tileset()->sharedFromThis());
        }
    }

    auto changeMapObjectCommand = new DetachObjects(currentMapDocument, templateInstances);

    // Add any missing tileset used by the templates to the map map before detaching
    for (const SharedTileset &sharedTileset : std::as_const(sharedTilesets)) {
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

void AbstractObjectTool::rotateLeft()
{
    mapDocument()->rotateSelectedObjects(RotateLeft);
}

void AbstractObjectTool::rotateRight()
{
    mapDocument()->rotateSelectedObjects(RotateRight);
}

void AbstractObjectTool::raise()
{
    RaiseLowerHelper(mapScene()).raise();
}

void AbstractObjectTool::lower()
{
    RaiseLowerHelper(mapScene()).lower();
}

void AbstractObjectTool::raiseToTop()
{
    RaiseLowerHelper(mapScene()).raiseToTop();
}

void AbstractObjectTool::lowerToBottom()
{
    RaiseLowerHelper(mapScene()).lowerToBottom();
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
    menu.setToolTipsVisible(true);

    QAction *duplicateAction = menu.addAction(tr("Duplicate %n Object(s)", "", selectedObjects.size()),
                                              this, &AbstractObjectTool::duplicateObjects);
    QAction *removeAction = menu.addAction(tr("Remove %n Object(s)", "", selectedObjects.size()),
                                           this, &AbstractObjectTool::removeObjects);

    duplicateAction->setIcon(QIcon(QLatin1String(":/images/16/stock-duplicate-16.png")));
    removeAction->setIcon(QIcon(QLatin1String(":/images/16/edit-delete.png")));

    // Allow the currently selected collision shapes to be applied to all selected tiles in the tileset editor
    if (auto document = DocumentManager::instance()->currentDocument()) {
        if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
            menu.addSeparator();
            auto collisionMenu = menu.addMenu(tr("Apply Collision(s) to Selected Tiles"));
            collisionMenu->setEnabled(tilesetDocument->selectedTiles().count() > 1);
            collisionMenu->addAction(tr("Replace Existing Objects"), this, [this] { applyCollisionsToSelectedTiles(true); });
            collisionMenu->addAction(tr("Add Objects"), this, [this] { applyCollisionsToSelectedTiles(false); });
        }
    }

    bool anyTileObjectSelected = std::any_of(selectedObjects.begin(),
                                             selectedObjects.end(),
                                             isTileObject);

    if (anyTileObjectSelected) {
        menu.addSeparator();

        auto resetTileSizeAction = menu.addAction(tr("Reset Tile Size"), this, &AbstractObjectTool::resetTileSize);
        resetTileSizeAction->setEnabled(std::any_of(selectedObjects.begin(),
                                                    selectedObjects.end(),
                                                    isResizedTileObject));

        auto changeTileAction = menu.addAction(tr("Replace Tile"), this, &AbstractObjectTool::changeTile);
        changeTileAction->setEnabled(tile() && (!selectedObjects.first()->isTemplateBase() ||
                                                tile()->tileset()->isExternal()));
    }

    bool onlyRectangleObjectSelected = std::all_of(selectedObjects.begin(),
                                                   selectedObjects.end(),
                                                   isRectangleObject);
    if (onlyRectangleObjectSelected)
        menu.addAction(tr("Convert to Polygon"), this, &AbstractObjectTool::convertRectanglesToPolygons);

    menu.addSeparator();

    // Create action for replacing an object with a template
    auto replaceTemplateAction = menu.addAction(tr("Replace With Template"), this, &AbstractObjectTool::replaceObjectsWithTemplate);
    auto selectedTemplate = objectTemplate();

    if (selectedTemplate) {
        QString name = QFileInfo(selectedTemplate->fileName()).fileName();
        replaceTemplateAction->setText(tr("Replace With Template \"%1\"").arg(name));
    }
    if (!selectedTemplate || !mapDocument()->templateAllowed(selectedTemplate))
        replaceTemplateAction->setEnabled(false);

    if (selectedObjects.size() == 1) {
        MapObject *currentObject = selectedObjects.first();

        if (!(currentObject->isTemplateBase() || currentObject->isTemplateInstance())) {
            const Cell cell = selectedObjects.first()->cell();
            auto action = menu.addAction(tr("Save As Template"), this, &AbstractObjectTool::saveSelectedObject);

            if (!cell.isEmpty() && !cell.tileset()->isExternal()) {
                action->setEnabled(false);
                action->setToolTip(tr("Can't create template with embedded tileset"));
            }
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
        menu.addAction(tr("Detach"), this, &AbstractObjectTool::detachSelectedObjects);

        auto resetToTemplateAction = menu.addAction(tr("Reset Template Instance(s)"), this, &AbstractObjectTool::resetInstances);
        resetToTemplateAction->setEnabled(std::any_of(selectedObjects.begin(),
                                                      selectedObjects.end(),
                                                      isChangedTemplateInstance));
    }

    menu.addSeparator();
    menu.addAction(tr("Flip Horizontally"), this, &AbstractObjectTool::flipHorizontally, Qt::Key_X);
    menu.addAction(tr("Flip Vertically"), this, &AbstractObjectTool::flipVertically, Qt::Key_Y);

    ObjectGroup *sameObjectGroup = RaiseLowerHelper::sameObjectGroup(selectedObjects);
    if (sameObjectGroup && sameObjectGroup->drawOrder() == ObjectGroup::IndexOrder) {
        menu.addSeparator();
        menu.addAction(tr("Raise Object"), this, &AbstractObjectTool::raise, Qt::Key_PageUp);
        menu.addAction(tr("Lower Object"), this, &AbstractObjectTool::lower, Qt::Key_PageDown);
        menu.addAction(tr("Raise Object to Top"), this, &AbstractObjectTool::raiseToTop, Qt::Key_Home);
        menu.addAction(tr("Lower Object to Bottom"), this, &AbstractObjectTool::lowerToBottom, Qt::Key_End);
    }

    if (LayerIterator(mapDocument()->map(), Layer::ObjectGroupType).next()) {
        menu.addSeparator();
        QMenu *moveToLayerMenu = menu.addMenu(tr("Move %n Object(s) to Layer",
                                                 "", selectedObjects.size()));

        auto actionHandler = MapDocumentActionHandler::instance();
        actionHandler->populateMoveToLayerMenu(moveToLayerMenu, sameObjectGroup);
    }

    menu.addSeparator();
    QIcon propIcon(QLatin1String(":images/16/document-properties.png"));
    QAction *propertiesAction = menu.addAction(propIcon,
                                               tr("Object &Properties..."));

    Utils::setThemeIcon(removeAction, "edit-delete");
    Utils::setThemeIcon(propertiesAction, "document-properties");

    ActionManager::applyMenuExtensions(&menu, MenuIds::mapViewObjects);

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
        const auto selectedObjectsCopy = selectedObjects;
        mapDocument()->moveObjectsToGroup(selectedObjects, objectGroup);
        mapDocument()->setSelectedObjects(selectedObjectsCopy);
    }
}

void AbstractObjectTool::setActionsEnabled(bool enabled)
{
    mFlipHorizontal->setEnabled(enabled);
    mFlipVertical->setEnabled(enabled);
    mRotateLeft->setEnabled(enabled);
    mRotateRight->setEnabled(enabled);
}

#include "moc_abstractobjecttool.cpp"
