/*
 * mapdocumentactionhandler.cpp
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com
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

#include "mapdocumentactionhandler.h"

#include "addremovelayer.h"
#include "addremovemapobject.h"
#include "changeselectedarea.h"
#include "clipboardmanager.h"
#include "documentmanager.h"
#include "erasetiles.h"
#include "map.h"
#include "mapdocument.h"
#include "mapobject.h"
#include "maprenderer.h"
#include "mapview.h"
#include "objectgroup.h"
#include "tilelayer.h"
#include "utils.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCursor>
#include <QMenu>
#include <QtCore/qmath.h>

using namespace Tiled;
using namespace Tiled::Internal;

MapDocumentActionHandler *MapDocumentActionHandler::mInstance;

MapDocumentActionHandler::MapDocumentActionHandler(QObject *parent)
    : QObject(parent)
    , mMapDocument(nullptr)
{
    Q_ASSERT(!mInstance);
    mInstance = this;

    mActionSelectAll = new QAction(this);
    mActionSelectAll->setShortcuts(QKeySequence::SelectAll);
    mActionSelectInverse = new QAction(this);
    mActionSelectInverse->setShortcut(tr("Ctrl+I"));
    mActionSelectNone = new QAction(this);
    mActionSelectNone->setShortcut(tr("Ctrl+Shift+A"));

    mActionCropToSelection = new QAction(this);

    QIcon addTileLayerIcon(QLatin1String(":/images/16x16/layer-tile.png"));
    QIcon addObjectLayerIcon(QLatin1String(":/images/16x16/layer-object.png"));
    QIcon addImageLayerIcon(QLatin1String(":/images/16x16/layer-image.png"));

    addTileLayerIcon.addFile(QLatin1String(":/images/32x32/layer-tile.png"));
    addObjectLayerIcon.addFile(QLatin1String(":/images/32x32/layer-object.png"));

    mActionAddTileLayer = new QAction(this);
    mActionAddTileLayer->setIcon(addTileLayerIcon);
    mActionAddObjectGroup = new QAction(this);
    mActionAddObjectGroup->setIcon(addObjectLayerIcon);
    mActionAddImageLayer = new QAction(this);
    mActionAddImageLayer->setIcon(addImageLayerIcon);

    mActionDuplicateLayer = new QAction(this);
    mActionDuplicateLayer->setShortcut(tr("Ctrl+Shift+D"));
    mActionDuplicateLayer->setIcon(
            QIcon(QLatin1String(":/images/16x16/stock-duplicate-16.png")));

    mActionMergeLayerDown = new QAction(this);

    mActionRemoveLayer = new QAction(this);
    mActionRemoveLayer->setIcon(
            QIcon(QLatin1String(":/images/16x16/edit-delete.png")));

    mActionLayerViaCopy = new QAction(this);
    mActionLayerViaCopy->setShortcut(tr("Ctrl+J"));

    mActionLayerViaCut = new QAction(this);
    mActionLayerViaCut->setShortcut(tr("Ctrl+Shift+J"));

    mActionSelectPreviousLayer = new QAction(this);
    mActionSelectPreviousLayer->setShortcut(tr("Ctrl+PgUp"));

    mActionSelectNextLayer = new QAction(this);
    mActionSelectNextLayer->setShortcut(tr("Ctrl+PgDown"));

    mActionMoveLayerUp = new QAction(this);
    mActionMoveLayerUp->setShortcut(tr("Ctrl+Shift+Up"));
    mActionMoveLayerUp->setIcon(
            QIcon(QLatin1String(":/images/16x16/go-up.png")));

    mActionMoveLayerDown = new QAction(this);
    mActionMoveLayerDown->setShortcut(tr("Ctrl+Shift+Down"));
    mActionMoveLayerDown->setIcon(
            QIcon(QLatin1String(":/images/16x16/go-down.png")));

    mActionToggleOtherLayers = new QAction(this);
    mActionToggleOtherLayers->setShortcut(tr("Ctrl+Shift+H"));
    mActionToggleOtherLayers->setIcon(
            QIcon(QLatin1String(":/images/16x16/show_hide_others.png")));

    mActionLayerProperties = new QAction(this);
    mActionLayerProperties->setIcon(
            QIcon(QLatin1String(":images/16x16/document-properties.png")));

    mActionDuplicateObjects = new QAction(this);
    mActionDuplicateObjects->setIcon(QIcon(QLatin1String(":/images/16x16/stock-duplicate-16.png")));

    mActionRemoveObjects = new QAction(this);
    mActionRemoveObjects->setIcon(QIcon(QLatin1String(":/images/16x16/edit-delete.png")));

    Utils::setThemeIcon(mActionRemoveLayer, "edit-delete");
    Utils::setThemeIcon(mActionMoveLayerUp, "go-up");
    Utils::setThemeIcon(mActionMoveLayerDown, "go-down");
    Utils::setThemeIcon(mActionLayerProperties, "document-properties");
    Utils::setThemeIcon(mActionRemoveObjects, "edit-delete");

    connect(mActionSelectAll, SIGNAL(triggered()), SLOT(selectAll()));
    connect(mActionSelectInverse, SIGNAL(triggered()), SLOT(selectInverse()));
    connect(mActionSelectNone, SIGNAL(triggered()), SLOT(selectNone()));
    connect(mActionCropToSelection, SIGNAL(triggered()),
            SLOT(cropToSelection()));
    connect(mActionAddTileLayer, SIGNAL(triggered()), SLOT(addTileLayer()));
    connect(mActionAddObjectGroup, SIGNAL(triggered()),
            SLOT(addObjectGroup()));
    connect(mActionAddImageLayer, SIGNAL(triggered()), SLOT(addImageLayer()));
    connect(mActionLayerViaCopy, &QAction::triggered, this, &MapDocumentActionHandler::layerViaCopy);
    connect(mActionLayerViaCut, &QAction::triggered, this, &MapDocumentActionHandler::layerViaCut);
    connect(mActionDuplicateLayer, SIGNAL(triggered()),
            SLOT(duplicateLayer()));
    connect(mActionMergeLayerDown, SIGNAL(triggered()),
            SLOT(mergeLayerDown()));
    connect(mActionSelectPreviousLayer, SIGNAL(triggered()),
            SLOT(selectPreviousLayer()));
    connect(mActionSelectNextLayer, SIGNAL(triggered()),
            SLOT(selectNextLayer()));
    connect(mActionRemoveLayer, SIGNAL(triggered()), SLOT(removeLayer()));
    connect(mActionMoveLayerUp, SIGNAL(triggered()), SLOT(moveLayerUp()));
    connect(mActionMoveLayerDown, SIGNAL(triggered()), SLOT(moveLayerDown()));
    connect(mActionToggleOtherLayers, SIGNAL(triggered()),
            SLOT(toggleOtherLayers()));
    connect(mActionLayerProperties, SIGNAL(triggered()),
            SLOT(layerProperties()));

    connect(mActionDuplicateObjects, SIGNAL(triggered()), SLOT(duplicateObjects()));
    connect(mActionRemoveObjects, SIGNAL(triggered()), SLOT(removeObjects()));

    updateActions();
    retranslateUi();
}

MapDocumentActionHandler::~MapDocumentActionHandler()
{
    mInstance = nullptr;
}

void MapDocumentActionHandler::retranslateUi()
{
    mActionSelectAll->setText(tr("Select &All"));
    mActionSelectInverse->setText(tr("Invert S&election"));
    mActionSelectNone->setText(tr("Select &None"));
    mActionCropToSelection->setText(tr("&Crop to Selection"));
    mActionAddTileLayer->setText(tr("&Tile Layer"));
    mActionAddObjectGroup->setText(tr("&Object Layer"));
    mActionAddImageLayer->setText(tr("&Image Layer"));
    mActionLayerViaCopy->setText(tr("Layer via Copy"));
    mActionLayerViaCut->setText(tr("Layer via Cut"));
    mActionDuplicateLayer->setText(tr("&Duplicate Layer"));
    mActionMergeLayerDown->setText(tr("&Merge Layer Down"));
    mActionRemoveLayer->setText(tr("&Remove Layer"));
    mActionSelectPreviousLayer->setText(tr("Select Pre&vious Layer"));
    mActionSelectNextLayer->setText(tr("Select &Next Layer"));
    mActionMoveLayerUp->setText(tr("R&aise Layer"));
    mActionMoveLayerDown->setText(tr("&Lower Layer"));
    mActionToggleOtherLayers->setText(tr("Show/&Hide all Other Layers"));
    mActionLayerProperties->setText(tr("Layer &Properties..."));
}

void MapDocumentActionHandler::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument == mapDocument)
        return;

    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;
    updateActions();

    if (mMapDocument) {
        connect(mapDocument, SIGNAL(layerRemoved(int)),
                SLOT(updateActions()));
        connect(mapDocument, SIGNAL(currentLayerIndexChanged(int)),
                SLOT(updateActions()));
        connect(mapDocument, SIGNAL(selectedAreaChanged(QRegion,QRegion)),
                SLOT(updateActions()));
        connect(mapDocument, SIGNAL(selectedObjectsChanged()),
                SLOT(updateActions()));
    }

    emit mapDocumentChanged(mMapDocument);
}

/**
 * Creates the new layer menu, which is used in several places.
 */
QMenu *MapDocumentActionHandler::createNewLayerMenu(QWidget *parent) const
{
    QMenu *newLayerMenu = new QMenu(tr("&New"), parent);

    newLayerMenu->setIcon(QIcon(QLatin1String(":/images/16x16/document-new.png")));
    Utils::setThemeIcon(newLayerMenu, "document-new");

    newLayerMenu->addAction(actionAddTileLayer());
    newLayerMenu->addAction(actionAddObjectGroup());
    newLayerMenu->addAction(actionAddImageLayer());
    newLayerMenu->addSeparator();
    newLayerMenu->addAction(actionLayerViaCopy());
    newLayerMenu->addAction(actionLayerViaCut());

    return newLayerMenu;
}

void MapDocumentActionHandler::cut()
{
    if (!mMapDocument)
        return;

    Layer *currentLayer = mMapDocument->currentLayer();
    if (!currentLayer)
        return;

    TileLayer *tileLayer = dynamic_cast<TileLayer*>(currentLayer);
    const QRegion &selectedArea = mMapDocument->selectedArea();
    const QList<MapObject*> &selectedObjects = mMapDocument->selectedObjects();

    copy();

    QUndoStack *stack = mMapDocument->undoStack();
    stack->beginMacro(tr("Cut"));

    if (tileLayer && !selectedArea.isEmpty()) {
        stack->push(new EraseTiles(mMapDocument, tileLayer, selectedArea));
    } else if (!selectedObjects.isEmpty()) {
        foreach (MapObject *mapObject, selectedObjects)
            stack->push(new RemoveMapObject(mMapDocument, mapObject));
    }

    selectNone();

    stack->endMacro();
}

void MapDocumentActionHandler::copy()
{
    if (mMapDocument)
        ClipboardManager::instance()->copySelection(mMapDocument);
}

void MapDocumentActionHandler::delete_()
{
    if (!mMapDocument)
        return;

    Layer *currentLayer = mMapDocument->currentLayer();
    if (!currentLayer)
        return;

    TileLayer *tileLayer = dynamic_cast<TileLayer*>(currentLayer);
    const QRegion &selectedArea = mMapDocument->selectedArea();
    const QList<MapObject*> &selectedObjects = mMapDocument->selectedObjects();

    QUndoStack *undoStack = mMapDocument->undoStack();
    undoStack->beginMacro(tr("Delete"));

    if (tileLayer && !selectedArea.isEmpty()) {
        undoStack->push(new EraseTiles(mMapDocument, tileLayer, selectedArea));
    } else if (!selectedObjects.isEmpty()) {
        foreach (MapObject *mapObject, selectedObjects)
            undoStack->push(new RemoveMapObject(mMapDocument, mapObject));
    }

    selectNone();
    undoStack->endMacro();
}

void MapDocumentActionHandler::selectAll()
{
    if (!mMapDocument)
        return;

    Layer *layer = mMapDocument->currentLayer();
    if (!layer)
        return;

    if (TileLayer *tileLayer = layer->asTileLayer()) {
        QRect all(tileLayer->x(), tileLayer->y(),
                  tileLayer->width(), tileLayer->height());

        if (mMapDocument->selectedArea() == all)
            return;

        QUndoCommand *command = new ChangeSelectedArea(mMapDocument, all);
        mMapDocument->undoStack()->push(command);
    } else if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
        mMapDocument->setSelectedObjects(objectGroup->objects());
    }
}

void MapDocumentActionHandler::selectInverse()
{
    if (!mMapDocument)
        return;

    Layer *layer = mMapDocument->currentLayer();
    if (!layer)
        return;

    if (TileLayer *tileLayer = layer->asTileLayer()) {
        QRegion all(tileLayer->bounds());

        QUndoCommand *command = new ChangeSelectedArea(mMapDocument, all - mMapDocument->selectedArea());
        mMapDocument->undoStack()->push(command);
    } else if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
        const auto &allObjects = objectGroup->objects();
        const auto &selectedObjects = mMapDocument->selectedObjects();
        QList<MapObject*> notSelectedObjects;

        for (auto mapObject : allObjects)
            if (!selectedObjects.contains(mapObject))
                notSelectedObjects.append(mapObject);

        mMapDocument->setSelectedObjects(notSelectedObjects);
    }
}

void MapDocumentActionHandler::selectNone()
{
    if (!mMapDocument)
        return;

    Layer *layer = mMapDocument->currentLayer();
    if (!layer)
        return;

    if (layer->asTileLayer()) {
        if (mMapDocument->selectedArea().isEmpty())
            return;

        QUndoCommand *command = new ChangeSelectedArea(mMapDocument, QRegion());
        mMapDocument->undoStack()->push(command);
    } else if (layer->asObjectGroup()) {
        mMapDocument->setSelectedObjects(QList<MapObject*>());
    }
}

void MapDocumentActionHandler::copyPosition()
{
    const MapView *view = DocumentManager::instance()->currentMapView();
    if (!view)
        return;

    const QPoint globalPos = QCursor::pos();
    const QPoint viewportPos = view->viewport()->mapFromGlobal(globalPos);
    const QPointF scenePos = view->mapToScene(viewportPos);

    const MapRenderer *renderer = mapDocument()->renderer();
    const QPointF tilePos = renderer->screenToTileCoords(scenePos);
    const int x = qFloor(tilePos.x());
    const int y = qFloor(tilePos.y());

    QApplication::clipboard()->setText(QString::number(x) +
                                       QLatin1String(", ") +
                                       QString::number(y));
}

void MapDocumentActionHandler::cropToSelection()
{
    if (!mMapDocument)
        return;

    const QRect bounds = mMapDocument->selectedArea().boundingRect();
    if (bounds.isNull())
        return;

    mMapDocument->resizeMap(bounds.size(), -bounds.topLeft(), true);
}

void MapDocumentActionHandler::addTileLayer()
{
    if (mMapDocument)
        mMapDocument->addLayer(Layer::TileLayerType);
}

void MapDocumentActionHandler::addObjectGroup()
{
    if (mMapDocument)
        mMapDocument->addLayer(Layer::ObjectGroupType);
}

void MapDocumentActionHandler::addImageLayer()
{
     if (mMapDocument)
         mMapDocument->addLayer(Layer::ImageLayerType);
}

void MapDocumentActionHandler::layerVia(MapDocumentActionHandler::LayerViaVariant variant)
{
    if (!mMapDocument)
        return;

    auto *currentLayer = mMapDocument->currentLayer();
    Layer *newLayer = nullptr;
    QRegion selectedArea;
    TileLayer *sourceLayer = nullptr;
    QList<MapObject*> selectedObjects;
    QList<MapObject*> newObjects;
    const QString name = variant == ViaCopy ? tr("Layer via Copy") : tr("Layer via Cut");

    switch (currentLayer->layerType()) {
    case Layer::TileLayerType: {
        selectedArea = mMapDocument->selectedArea();
        if (selectedArea.isEmpty())
            return;

        auto map = mMapDocument->map();
        sourceLayer = static_cast<TileLayer*>(currentLayer);
        auto newTileLayer = new TileLayer(name, 0, 0, map->width(), map->height());
        newTileLayer->setCells(0, 0, sourceLayer, selectedArea);

        newLayer = newTileLayer;
        break;
    }
    case Layer::ObjectGroupType: {
        selectedObjects = mMapDocument->selectedObjects();
        if (selectedObjects.isEmpty())
            return;

        auto currentObjectGroup = static_cast<ObjectGroup*>(currentLayer);
        auto newObjectGroup = new ObjectGroup(name, 0, 0);
        newObjectGroup->setDrawOrder(currentObjectGroup->drawOrder());
        newObjectGroup->setColor(currentObjectGroup->color());

        for (MapObject *mapObject : selectedObjects) {
            MapObject *clone = mapObject->clone();
            if (variant == ViaCopy)
                clone->resetId();
            newObjects.append(clone);
            newObjectGroup->addObject(clone);
        }

        newLayer = newObjectGroup;
        break;
    }
    default:
        return;
    }

    auto newLayerIndex = mMapDocument->currentLayerIndex() + 1;
    auto addLayer = new AddLayer(mMapDocument, newLayerIndex, newLayer);
    addLayer->setText(name);

    auto undoStack = mMapDocument->undoStack();

    if (variant == ViaCopy) {
        undoStack->push(addLayer);
    } else {
        undoStack->beginMacro(name);
        undoStack->push(addLayer);

        switch (currentLayer->layerType()) {
        case Layer::TileLayerType: {
            undoStack->push(new EraseTiles(mMapDocument, sourceLayer, selectedArea));
            break;
        }
        case Layer::ObjectGroupType:
            for (MapObject *oldObject : selectedObjects)
                undoStack->push(new RemoveMapObject(mMapDocument, oldObject));
            break;
        default:
            Q_ASSERT(false);
            break;
        }

        undoStack->endMacro();
    }

    mMapDocument->setCurrentLayerIndex(newLayerIndex);

    if (!newObjects.isEmpty())
        mMapDocument->setSelectedObjects(newObjects);
}

void MapDocumentActionHandler::duplicateLayer()
{
    if (mMapDocument)
        mMapDocument->duplicateLayer();
}

void MapDocumentActionHandler::mergeLayerDown()
{
    if (mMapDocument)
        mMapDocument->mergeLayerDown();
}

void MapDocumentActionHandler::selectPreviousLayer()
{
    if (mMapDocument) {
        const int currentLayer = mMapDocument->currentLayerIndex();
        if (currentLayer < mMapDocument->map()->layerCount() - 1)
            mMapDocument->setCurrentLayerIndex(currentLayer + 1);
    }
}

void MapDocumentActionHandler::selectNextLayer()
{
    if (mMapDocument) {
        const int currentLayer = mMapDocument->currentLayerIndex();
        if (currentLayer > 0)
            mMapDocument->setCurrentLayerIndex(currentLayer - 1);
    }
}

void MapDocumentActionHandler::moveLayerUp()
{
    if (mMapDocument)
        mMapDocument->moveLayerUp(mMapDocument->currentLayerIndex());
}

void MapDocumentActionHandler::moveLayerDown()
{
    if (mMapDocument)
        mMapDocument->moveLayerDown(mMapDocument->currentLayerIndex());
}

void MapDocumentActionHandler::removeLayer()
{
    if (mMapDocument)
        mMapDocument->removeLayer(mMapDocument->currentLayerIndex());
}

void MapDocumentActionHandler::toggleOtherLayers()
{
    if (mMapDocument)
        mMapDocument->toggleOtherLayers(mMapDocument->currentLayerIndex());
}

void MapDocumentActionHandler::layerProperties()
{
    if (mMapDocument) {
        mMapDocument->setCurrentObject(mMapDocument->currentLayer());
        emit mMapDocument->editCurrentObject();
    }
}

void MapDocumentActionHandler::duplicateObjects()
{
    if (mMapDocument)
        mMapDocument->duplicateObjects(mMapDocument->selectedObjects());
}

void MapDocumentActionHandler::removeObjects()
{
    if (mMapDocument)
        mMapDocument->removeObjects(mMapDocument->selectedObjects());
}

void MapDocumentActionHandler::moveObjectsToGroup(ObjectGroup *objectGroup)
{
    if (mMapDocument) {
        mMapDocument->moveObjectsToGroup(mMapDocument->selectedObjects(),
                                         objectGroup);
    }
}

void MapDocumentActionHandler::updateActions()
{
    Map *map = nullptr;
    int currentLayerIndex = -1;
    Layer *currentLayer = nullptr;
    QRegion selection;
    int selectedObjectsCount = 0;
    bool canMergeDown = false;

    if (mMapDocument) {
        map = mMapDocument->map();
        currentLayerIndex = mMapDocument->currentLayerIndex();
        currentLayer = mMapDocument->currentLayer();
        selection = mMapDocument->selectedArea();
        selectedObjectsCount = mMapDocument->selectedObjects().count();

        if (currentLayerIndex > 0) {
            Layer *upper = map->layerAt(currentLayerIndex);
            Layer *lower = map->layerAt(currentLayerIndex - 1);
            canMergeDown = lower->canMergeWith(upper);
        }
    }

    mActionSelectAll->setEnabled(map);

    if (currentLayer) {
        if (currentLayer->asTileLayer()) {
            mActionSelectNone->setEnabled(!selection.isEmpty());
        } else if (currentLayer->asObjectGroup()) {
            mActionSelectNone->setEnabled(selectedObjectsCount  > 0);
        } else {
            mActionSelectNone->setEnabled(false);
        }
    } else {
        mActionSelectNone->setEnabled(false);
    }


    mActionCropToSelection->setEnabled(!selection.isEmpty());

    mActionAddTileLayer->setEnabled(map);
    mActionAddObjectGroup->setEnabled(map);
    mActionAddImageLayer->setEnabled(map);

    bool usableSelection = currentLayer && ((currentLayer->isObjectGroup() && selectedObjectsCount > 0) ||
                                            (currentLayer->isTileLayer() && !selection.isEmpty()));
    mActionLayerViaCopy->setEnabled(usableSelection);
    mActionLayerViaCut->setEnabled(usableSelection);

    const int layerCount = map ? map->layerCount() : 0;
    const bool hasPreviousLayer = currentLayerIndex >= 0
            && currentLayerIndex < layerCount - 1;
    const bool hasNextLayer = currentLayerIndex > 0;

    mActionDuplicateLayer->setEnabled(currentLayerIndex >= 0);
    mActionMergeLayerDown->setEnabled(canMergeDown);
    mActionSelectPreviousLayer->setEnabled(hasPreviousLayer);
    mActionSelectNextLayer->setEnabled(hasNextLayer);
    mActionMoveLayerUp->setEnabled(hasPreviousLayer);
    mActionMoveLayerDown->setEnabled(hasNextLayer);
    mActionToggleOtherLayers->setEnabled(layerCount > 1);
    mActionRemoveLayer->setEnabled(currentLayerIndex >= 0);
    mActionLayerProperties->setEnabled(currentLayerIndex >= 0);

    mActionDuplicateObjects->setEnabled(selectedObjectsCount > 0);
    mActionRemoveObjects->setEnabled(selectedObjectsCount > 0);

    QString duplicateText;
    QString removeText;

    if (selectedObjectsCount > 0) {
        duplicateText = tr("Duplicate %n Object(s)", "", selectedObjectsCount);
        removeText = tr("Remove %n Object(s)", "", selectedObjectsCount);
    } else {
        duplicateText = tr("Duplicate Objects");
        removeText = tr("Remove Objects");
    }

    mActionDuplicateObjects->setText(duplicateText);
    mActionRemoveObjects->setText(removeText);
}
