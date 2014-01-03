/*
 * tilecollisioneditor.cpp
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "tilecollisioneditor.h"

#include "addremovemapobject.h"
#include "editpolygontool.h"
#include "changetileobjectgroup.h"
#include "clipboardmanager.h"
#include "createobjecttool.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"
#include "mapscene.h"
#include "mapview.h"
#include "objectgroup.h"
#include "objectselectiontool.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "toolmanager.h"
#include "utils.h"
#include "zoomable.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QShortcut>
#include <QStatusBar>
#include <QToolBar>
#include <QUndoStack>
#include <QVBoxLayout>

using namespace Tiled;
using namespace Tiled::Internal;

TileCollisionEditor::TileCollisionEditor(QWidget *parent)
    : QMainWindow(parent)
    , mTile(0)
    , mMapDocument(0)
    , mMapScene(new MapScene(this))
    , mMapView(new MapView(this, MapView::NoStaticContents))
    , mToolManager(new ToolManager(this))
    , mApplyingChanges(false)
    , mSynchronizing(false)
{
    setObjectName(QLatin1String("TileCollisionEditor"));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setSpacing(0);
    layout->setMargin(5);

    mMapView->setScene(mMapScene);

    mMapView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mMapView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    CreateObjectTool *rectangleObjectsTool = new CreateObjectTool(
            CreateObjectTool::CreateRectangle, this);
    CreateObjectTool *ellipseObjectsTool = new CreateObjectTool(
            CreateObjectTool::CreateEllipse, this);
    CreateObjectTool *polygonObjectsTool = new CreateObjectTool(
            CreateObjectTool::CreatePolygon, this);
    CreateObjectTool *polylineObjectsTool = new CreateObjectTool(
            CreateObjectTool::CreatePolyline, this);

    QToolBar *toolBar = new QToolBar(this);
    toolBar->setMovable(false);
    toolBar->setFloatable(false);

    mToolManager = new ToolManager(this);
    toolBar->addAction(mToolManager->registerTool(new ObjectSelectionTool(this)));
    toolBar->addAction(mToolManager->registerTool(new EditPolygonTool(this)));
    toolBar->addAction(mToolManager->registerTool(rectangleObjectsTool));
    toolBar->addAction(mToolManager->registerTool(ellipseObjectsTool));
    toolBar->addAction(mToolManager->registerTool(polygonObjectsTool));
    toolBar->addAction(mToolManager->registerTool(polylineObjectsTool));

    setCentralWidget(mMapView);
    addToolBar(toolBar);

    mMapScene->setSelectedTool(mToolManager->selectedTool());
    connect(mToolManager, SIGNAL(selectedToolChanged(AbstractTool*)),
            SLOT(setSelectedTool(AbstractTool*)));

    QComboBox *zoomComboBox = new QComboBox;
    statusBar()->addPermanentWidget(zoomComboBox);

    Zoomable *zoomable = mMapView->zoomable();
    zoomable->connectToComboBox(zoomComboBox);

    QShortcut *undoShortcut = new QShortcut(QKeySequence::Undo, this);
    QShortcut *redoShortcut = new QShortcut(QKeySequence::Redo, this);
    QShortcut *cutShortcut = new QShortcut(QKeySequence::Cut, this);
    QShortcut *copyShortcut = new QShortcut(QKeySequence::Copy, this);
    QShortcut *pasteShortcut = new QShortcut(QKeySequence::Paste, this);
    QShortcut *deleteShortcut = new QShortcut(QKeySequence::Delete, this);

    connect(undoShortcut, SIGNAL(activated()), SLOT(undo()));
    connect(redoShortcut, SIGNAL(activated()), SLOT(redo()));
    connect(cutShortcut, SIGNAL(activated()), SLOT(cut()));
    connect(copyShortcut, SIGNAL(activated()), SLOT(copy()));
    connect(pasteShortcut, SIGNAL(activated()), SLOT(paste()));
    connect(deleteShortcut, SIGNAL(activated()), SLOT(delete_()));

    retranslateUi();
    resize(300, 300);
    Utils::restoreGeometry(this);
}

TileCollisionEditor::~TileCollisionEditor()
{
    setTile(0);
}

void TileCollisionEditor::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;

    if (mMapDocument) {
        connect(mMapDocument, SIGNAL(tileObjectGroupChanged(Tile*)),
                SLOT(tileObjectGroupChanged(Tile*)));
    }
}

void TileCollisionEditor::writeSettings()
{
    Utils::saveGeometry(this);
}

void TileCollisionEditor::setTile(Tile *tile)
{
    if (mTile == tile)
        return;

    mTile = tile;

    mMapScene->disableSelectedTool();
    MapDocument *previousDocument = mMapScene->mapDocument();

    if (tile) {
        mMapView->setEnabled(!mTile->tileset()->isExternal());

        Map *map = new Map(Map::Orthogonal, 1, 1, tile->width(), tile->height());
        map->addTileset(tile->tileset());

        TileLayer *tileLayer = new TileLayer(QString(), 0, 0, 1, 1);
        tileLayer->setCell(0, 0, Cell(tile));
        map->addLayer(tileLayer);

        ObjectGroup *objectGroup;
        if (tile->objectGroup())
            objectGroup = static_cast<ObjectGroup*>(tile->objectGroup()->clone());
        else
            objectGroup = new ObjectGroup;

        objectGroup->setDrawOrder(ObjectGroup::IndexOrder);
        map->addLayer(objectGroup);

        MapDocument *mapDocument = new MapDocument(map);
        mMapScene->setMapDocument(mapDocument);

        mToolManager->setMapDocument(mapDocument);
        mapDocument->setCurrentLayerIndex(1);

        mMapScene->enableSelectedTool();

        connect(mapDocument->undoStack(), SIGNAL(indexChanged(int)),
                SLOT(applyChanges()));
    } else {
        mMapView->setEnabled(false);
        mMapScene->setMapDocument(0);
        mToolManager->setMapDocument(0);
    }

    if (previousDocument) {
        previousDocument->undoStack()->disconnect(this);
        delete previousDocument;
    }
}

void TileCollisionEditor::closeEvent(QCloseEvent *event)
{
    QMainWindow::closeEvent(event);
    if (event->isAccepted())
        emit closed();
}

void TileCollisionEditor::setSelectedTool(AbstractTool *tool)
{
    mMapScene->disableSelectedTool();
    mMapScene->setSelectedTool(tool);
    mMapScene->enableSelectedTool();
}

void TileCollisionEditor::applyChanges()
{
    if (mSynchronizing)
        return;

    MapDocument *dummyDocument = mMapScene->mapDocument();
    Layer *objectGroup = dummyDocument->map()->layerAt(1);
    ObjectGroup *clonedGroup = static_cast<ObjectGroup*>(objectGroup->clone());

    QUndoStack *undoStack = mMapDocument->undoStack();
    mApplyingChanges = true;
    undoStack->push(new ChangeTileObjectGroup(mMapDocument, mTile, clonedGroup));
    mApplyingChanges = false;
}

void TileCollisionEditor::tileObjectGroupChanged(Tile *tile)
{
    if (mTile != tile)
        return;
    if (mApplyingChanges)
        return;

    mSynchronizing = true;

    MapDocument *dummyDocument = mMapScene->mapDocument();
    LayerModel *layerModel = dummyDocument->layerModel();
    dummyDocument->undoStack()->clear();

    delete layerModel->takeLayerAt(1);

    ObjectGroup *objectGroup;
    if (tile->objectGroup())
        objectGroup = static_cast<ObjectGroup*>(tile->objectGroup()->clone());
    else
        objectGroup = new ObjectGroup;

    objectGroup->setDrawOrder(ObjectGroup::IndexOrder);

    layerModel->insertLayer(1, objectGroup);
    dummyDocument->setCurrentLayerIndex(1);

    mSynchronizing = false;
}

void TileCollisionEditor::undo()
{
    if (mMapDocument)
        mMapDocument->undoStack()->undo();
}

void TileCollisionEditor::redo()
{
    if (mMapDocument)
        mMapDocument->undoStack()->redo();
}

void TileCollisionEditor::cut()
{
    if (!mTile)
        return;

    copy();
    delete_(Cut);
}

void TileCollisionEditor::copy()
{
    if (!mTile)
        return;

    MapDocument *dummyDocument = mMapScene->mapDocument();
    ClipboardManager::instance()->copySelection(dummyDocument);
}

void TileCollisionEditor::paste()
{
    if (!mTile)
        return;

    ClipboardManager *clipboardManager = ClipboardManager::instance();
    QScopedPointer<Map> map(clipboardManager->map());
    if (!map)
        return;

    // Clean up the tilesets, we're not interested in them (would make sense
    // to avoid loading them in the first place).
    qDeleteAll(map->tilesets());

    // We can currently only handle maps with a single layer
    if (map->layerCount() != 1)
        return;

    Layer *layer = map->layerAt(0);

    if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
        MapDocument *dummyDocument = mMapScene->mapDocument();
        clipboardManager->pasteObjectGroup(objectGroup,
                                           dummyDocument, mMapView,
                                           ClipboardManager::NoTileObjects);
    }
}

void TileCollisionEditor::delete_(Operation operation)
{
    if (!mTile)
        return;

    MapDocument *dummyDocument = mMapScene->mapDocument();
    const QList<MapObject*> &selectedObjects = dummyDocument->selectedObjects();
    if (selectedObjects.isEmpty())
        return;

    QUndoStack *undoStack = dummyDocument->undoStack();
    undoStack->beginMacro(operation == Delete ? tr("Delete") : tr("Cut"));

    foreach (MapObject *mapObject, selectedObjects)
        undoStack->push(new RemoveMapObject(dummyDocument, mapObject));

    undoStack->endMacro();
}

void TileCollisionEditor::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void TileCollisionEditor::retranslateUi()
{
    setWindowTitle(tr("Tile Collision Editor"));
}
