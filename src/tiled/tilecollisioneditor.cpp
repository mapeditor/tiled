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
#include "createobjecttool.h"
#include "createrectangleobjecttool.h"
#include "createellipseobjecttool.h"
#include "createpolygonobjecttool.h"
#include "createpolylineobjecttool.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"
#include "mapobject.h"
#include "mapscene.h"
#include "mapview.h"
#include "objectgroup.h"
#include "objectselectiontool.h"
#include "propertiesdock.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "tilesetdocument.h"
#include "toolmanager.h"
#include "utils.h"
#include "zoomable.h"

#include <QCloseEvent>
#include <QCoreApplication>
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
    , mTile(nullptr)
    , mTilesetDocument(nullptr)
    , mMapScene(new MapScene(this))
    , mMapView(new MapView(this, MapView::NoStaticContents))
    , mToolManager(new ToolManager(this))
    , mPropertiesDock(new PropertiesDock(this))
    , mApplyingChanges(false)
    , mSynchronizing(false)
{
    setObjectName(QLatin1String("TileCollisionEditor"));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setSpacing(0);
    layout->setMargin(5);

    // We re-use the PropertiesDock class in order to manipulate properties on
    // collision objects. This instance of the PropertiesDock is best left with
    // the docking features disabled though as it only belongs to the collision
    // editor.
    QDockWidget::DockWidgetFeatures features = QDockWidget::NoDockWidgetFeatures;
    mPropertiesDock->setFeatures(features);
    mPropertiesDock->setContextMenuPolicy(Qt::PreventContextMenu);
    addDockWidget(Qt::LeftDockWidgetArea, mPropertiesDock);

    mMapView->setScene(mMapScene);

    mMapView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mMapView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    CreateObjectTool *rectangleObjectsTool = new CreateRectangleObjectTool(this);
    CreateObjectTool *ellipseObjectsTool = new CreateEllipseObjectTool(this);
    CreateObjectTool *polygonObjectsTool = new CreatePolygonObjectTool(this);
    CreateObjectTool *polylineObjectsTool = new CreatePolylineObjectTool(this);;

    QToolBar *toolBar = new QToolBar(this);
    toolBar->setObjectName(QLatin1String("TileCollisionEditorToolBar"));
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setContextMenuPolicy(Qt::ActionsContextMenu);

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
    QShortcut *pasteInPlaceShortcut = new QShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+V"), this);
    QShortcut *deleteShortcut = new QShortcut(QKeySequence::Delete, this);
    QShortcut *deleteShortcut2 = new QShortcut(QKeySequence(Qt::Key_Backspace), this);

    connect(undoShortcut, SIGNAL(activated()), SLOT(undo()));
    connect(redoShortcut, SIGNAL(activated()), SLOT(redo()));
    connect(cutShortcut, SIGNAL(activated()), SLOT(cut()));
    connect(copyShortcut, SIGNAL(activated()), SLOT(copy()));
    connect(pasteShortcut, SIGNAL(activated()), SLOT(paste()));
    connect(pasteInPlaceShortcut, SIGNAL(activated()), SLOT(pasteInPlace()));
    connect(deleteShortcut, SIGNAL(activated()), SLOT(delete_()));
    connect(deleteShortcut2, SIGNAL(activated()), SLOT(delete_()));

    retranslateUi();
    resize(300, 300);
    Utils::restoreGeometry(this);
}

TileCollisionEditor::~TileCollisionEditor()
{
    setTile(nullptr);
}

void TileCollisionEditor::setTilesetDocument(TilesetDocument *tilesetDocument)
{
    if (mTilesetDocument)
        mTilesetDocument->disconnect(this);

    mTilesetDocument = tilesetDocument;

    if (mTilesetDocument) {
        connect(mTilesetDocument, SIGNAL(tileObjectGroupChanged(Tile*)),
                SLOT(tileObjectGroupChanged(Tile*)));

        connect(mTilesetDocument, &MapDocument::currentObjectChanged,
                this, &TileCollisionEditor::currentObjectChanged);
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
        Map *map = new Map(Map::Orthogonal, 1, 1, tile->width(), tile->height());
        map->addTileset(tile->sharedTileset());

        TileLayer *tileLayer = new TileLayer(QString(), 0, 0, 1, 1);
        tileLayer->setCell(0, 0, Cell(tile));
        map->addLayer(tileLayer);

        ObjectGroup *objectGroup;
        if (tile->objectGroup())
            objectGroup = static_cast<ObjectGroup*>(tile->objectGroup()->clone());
        else
            objectGroup = new ObjectGroup;

        objectGroup->setDrawOrder(ObjectGroup::IndexOrder);
        map->setNextObjectId(objectGroup->highestObjectId() + 1);
        map->addLayer(objectGroup);

        MapDocument *mapDocument = new MapDocument(map);
        mMapScene->setMapDocument(mapDocument);

        mToolManager->setMapDocument(mapDocument);
        mPropertiesDock->setDocument(mapDocument);

        mapDocument->setCurrentLayerIndex(1);

        mMapScene->enableSelectedTool();

        connect(mapDocument->undoStack(), &QUndoStack::indexChanged,
                this, &TileCollisionEditor::applyChanges);

        connect(mapDocument, &MapDocument::selectedObjectsChanged,
                this, &TileCollisionEditor::selectedObjectsChanged);

    } else {
        mMapView->setEnabled(false);
        mPropertiesDock->setEnabled(false);
        mMapScene->setMapDocument(nullptr);
        mToolManager->setMapDocument(nullptr);
        mPropertiesDock->setDocument(nullptr);
    }

    if (previousDocument) {
        // Explicitly disconnect early from this signal, since it can get fired
        // from the QUndoStack destructor.
        disconnect(previousDocument->undoStack(), &QUndoStack::indexChanged,
                   this, &TileCollisionEditor::applyChanges);

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

    QUndoStack *undoStack = mTilesetDocument->undoStack();
    mApplyingChanges = true;
    undoStack->push(new ChangeTileObjectGroup(mTilesetDocument, mTile, clonedGroup));
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

void TileCollisionEditor::currentObjectChanged(Object *object)
{
    // If a tile object is selected, edit the collision shapes for that tile
    if (object && object->typeId() == Object::MapObjectType) {
        const Cell &cell = static_cast<MapObject*>(object)->cell();
        if (cell.tile)
            setTile(cell.tile);
    }
}

void TileCollisionEditor::undo()
{
    if (mTilesetDocument)
        mTilesetDocument->undoStack()->undo();
}

void TileCollisionEditor::redo()
{
    if (mTilesetDocument)
        mTilesetDocument->undoStack()->redo();
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
    paste(ClipboardManager::PasteDefault);
}

void TileCollisionEditor::pasteInPlace()
{
    paste(ClipboardManager::PasteInPlace);
}

void TileCollisionEditor::paste(ClipboardManager::PasteFlags flags)
{
    if (!mTile)
        return;

    ClipboardManager *clipboardManager = ClipboardManager::instance();
    QScopedPointer<Map> map(clipboardManager->map());
    if (!map)
        return;

    // We can currently only handle maps with a single layer
    if (map->layerCount() != 1)
        return;

    Layer *layer = map->layerAt(0);

    if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
        MapDocument *dummyDocument = mMapScene->mapDocument();
        clipboardManager->pasteObjectGroup(objectGroup,
                                           dummyDocument, mMapView,
                                           flags | ClipboardManager::PasteNoTileObjects);
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

    for (MapObject *mapObject : selectedObjects)
        undoStack->push(new RemoveMapObject(dummyDocument, mapObject));

    undoStack->endMacro();
}

void TileCollisionEditor::selectedObjectsChanged()
{
    MapDocument *dummyDocument = mMapScene->mapDocument();
    if (dummyDocument->selectedObjects().isEmpty())
        dummyDocument->setCurrentObject(dummyDocument->map()->layerAt(1));
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
