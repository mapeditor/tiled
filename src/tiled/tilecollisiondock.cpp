/*
 * tilecollisiondock.cpp
 * Copyright 2013-2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "tilecollisiondock.h"

#include "addremovemapobject.h"
#include "changetileobjectgroup.h"
#include "createellipseobjecttool.h"
#include "createobjecttool.h"
#include "createpointobjecttool.h"
#include "createpolygonobjecttool.h"
#include "createrectangleobjecttool.h"
#include "createtemplatetool.h"
#include "editpolygontool.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"
#include "mapobject.h"
#include "mapscene.h"
#include "mapview.h"
#include "objectgroup.h"
#include "objectselectiontool.h"
#include "objectsview.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "tilesetdocument.h"
#include "toolmanager.h"
#include "utils.h"
#include "zoomable.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QShortcut>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QUndoStack>
#include <QVBoxLayout>

namespace Tiled {

TileCollisionDock::TileCollisionDock(QWidget *parent)
    : QDockWidget(parent)
    , mTile(nullptr)
    , mTilesetDocument(nullptr)
    , mMapScene(new MapScene(this))
    , mMapView(new MapView(this, MapView::NoStaticContents))
    , mObjectsView(new ObjectsView(this))
    , mToolManager(new ToolManager(this))
    , mApplyingChanges(false)
    , mSynchronizing(false)
    , mHasSelectedObjects(false)
{
    setObjectName(QLatin1String("tileCollisionDock"));

    mMapView->setScene(mMapScene);

    mMapView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    mMapView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mMapView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    mObjectsView->setRootIsDecorated(false);

    CreateObjectTool *rectangleObjectsTool = new CreateRectangleObjectTool(this);
    CreateObjectTool *pointObjectsTool = new CreatePointObjectTool(this);
    CreateObjectTool *ellipseObjectsTool = new CreateEllipseObjectTool(this);
    CreateObjectTool *polygonObjectsTool = new CreatePolygonObjectTool(this);
    CreateObjectTool *templatesTool = new CreateTemplateTool(this);

    QToolBar *toolBar = new QToolBar(this);
    toolBar->setObjectName(QLatin1String("TileCollisionDockToolBar"));
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setContextMenuPolicy(Qt::ActionsContextMenu);

    mToolManager = new ToolManager(this);
    toolBar->addAction(mToolManager->registerTool(new ObjectSelectionTool(this)));
    toolBar->addAction(mToolManager->registerTool(new EditPolygonTool(this)));
    toolBar->addAction(mToolManager->registerTool(rectangleObjectsTool));
    toolBar->addAction(mToolManager->registerTool(pointObjectsTool));
    toolBar->addAction(mToolManager->registerTool(ellipseObjectsTool));
    toolBar->addAction(mToolManager->registerTool(polygonObjectsTool));
    toolBar->addAction(mToolManager->registerTool(templatesTool));

    mActionMoveUp = new QAction(this);
    mActionMoveUp->setIcon(QIcon(QLatin1String(":/images/16x16/go-up.png")));
    mActionMoveUp->setEnabled(false);
    mActionMoveDown = new QAction(this);
    mActionMoveDown->setIcon(QIcon(QLatin1String(":/images/16x16/go-down.png")));
    mActionMoveDown->setEnabled(false);

    Utils::setThemeIcon(mActionMoveUp, "go-up");
    Utils::setThemeIcon(mActionMoveDown, "go-down");

    QToolBar *objectsToolBar = new QToolBar(this);
    objectsToolBar->setMovable(false);
    objectsToolBar->setFloatable(false);
    objectsToolBar->setIconSize(Utils::smallIconSize());
    objectsToolBar->addAction(mActionMoveUp);
    objectsToolBar->addAction(mActionMoveDown);

    auto objectsWidget = new QWidget;
    auto objectsVertical = new QVBoxLayout(objectsWidget);
    objectsVertical->setSpacing(0);
    objectsVertical->setMargin(0);
    objectsVertical->addWidget(mObjectsView);
    objectsVertical->addWidget(objectsToolBar);

    auto widget = new QWidget(this);
    auto vertical = new QVBoxLayout(widget);
    vertical->setSpacing(0);
    vertical->setMargin(0);

    auto horizontal = new QHBoxLayout;
    horizontal->addWidget(toolBar, 1);

    auto splitter = new QSplitter;
    splitter->addWidget(mMapView);
    splitter->addWidget(objectsWidget);

    vertical->addLayout(horizontal);
    vertical->addWidget(splitter);

    setWidget(widget);

    mMapScene->setSelectedTool(mToolManager->selectedTool());
    connect(mToolManager, &ToolManager::selectedToolChanged,
            this, &TileCollisionDock::setSelectedTool);
    connect(mToolManager, &ToolManager::statusInfoChanged,
            this, &TileCollisionDock::statusInfoChanged);

    QComboBox *zoomComboBox = new QComboBox;
    horizontal->addWidget(zoomComboBox);

    Zoomable *zoomable = mMapView->zoomable();
    zoomable->setComboBox(zoomComboBox);

    auto selectAllShortcut = new QShortcut(Qt::CTRL + Qt::Key_A, this, nullptr, nullptr, Qt::WidgetWithChildrenShortcut);
    connect(selectAllShortcut, &QShortcut::activated, this, &TileCollisionDock::selectAll);

    connect(mActionMoveUp, &QAction::triggered, this, &TileCollisionDock::moveObjectsUp);
    connect(mActionMoveDown, &QAction::triggered, this, &TileCollisionDock::moveObjectsDown);

    retranslateUi();
}

TileCollisionDock::~TileCollisionDock()
{
    setTile(nullptr);
}

void TileCollisionDock::setTilesetDocument(TilesetDocument *tilesetDocument)
{
    if (mTilesetDocument)
        mTilesetDocument->disconnect(this);

    mTilesetDocument = tilesetDocument;

    if (mTilesetDocument) {
        connect(mTilesetDocument, &TilesetDocument::tileObjectGroupChanged,
                this, &TileCollisionDock::tileObjectGroupChanged);
        connect(mTilesetDocument, &TilesetDocument::tilesetTileOffsetChanged,
                this, &TileCollisionDock::tilesetTileOffsetChanged);
    }
}

void TileCollisionDock::setTile(Tile *tile)
{
    if (mTile == tile)
        return;

    mTile = tile;

    mMapScene->disableSelectedTool();
    auto previousDocument = mDummyMapDocument;

    mMapView->setEnabled(tile);

    if (tile) {
        Map::Orientation orientation = Map::Orthogonal;
        QSize tileSize = tile->size();

        if (tile->tileset()->orientation() == Tileset::Isometric) {
            orientation = Map::Isometric;
            tileSize = tile->tileset()->gridSize();
        }

        std::unique_ptr<Map> map { new Map(orientation, 1, 1, tileSize.width(), tileSize.height()) };
        map->addTileset(tile->sharedTileset());

        TileLayer *tileLayer = new TileLayer(QString(), 0, 0, 1, 1);
        tileLayer->setCell(0, 0, Cell(tile));
        tileLayer->setOffset(-tile->offset());  // undo tile offset
        map->addLayer(tileLayer);

        ObjectGroup *objectGroup;
        if (tile->objectGroup())
            objectGroup = tile->objectGroup()->clone();
        else
            objectGroup = new ObjectGroup;

        objectGroup->setDrawOrder(ObjectGroup::IndexOrder);
        map->setNextObjectId(objectGroup->highestObjectId() + 1);
        map->addLayer(objectGroup);

        mDummyMapDocument = MapDocumentPtr::create(map.release());
        mDummyMapDocument->setAllowHidingObjects(false);
        mDummyMapDocument->setAllowTileObjects(false);
        mDummyMapDocument->setCurrentLayer(objectGroup);

        mMapScene->setMapDocument(mDummyMapDocument.data());
        mObjectsView->setMapDocument(mDummyMapDocument.data());
        mObjectsView->setRootIndex(mObjectsView->layerViewIndex(objectGroup));
        mToolManager->setMapDocument(mDummyMapDocument.data());

        mMapScene->enableSelectedTool();

        connect(mDummyMapDocument->undoStack(), &QUndoStack::indexChanged,
                this, &TileCollisionDock::applyChanges);

        connect(mDummyMapDocument.data(), &MapDocument::selectedObjectsChanged,
                this, &TileCollisionDock::selectedObjectsChanged);

    } else {
        mDummyMapDocument.clear();
        mMapScene->setMapDocument(nullptr);
        mObjectsView->setMapDocument(nullptr);
        mToolManager->setMapDocument(nullptr);
    }


    emit dummyMapDocumentChanged(mDummyMapDocument.data());

    setHasSelectedObjects(false);

    if (previousDocument) {
        // Explicitly disconnect early from this signal, since it can get fired
        // from the QUndoStack destructor.
        disconnect(previousDocument->undoStack(), &QUndoStack::indexChanged,
                   this, &TileCollisionDock::applyChanges);
    }
}

void TileCollisionDock::setSelectedTool(AbstractTool *tool)
{
    mMapScene->disableSelectedTool();
    mMapScene->setSelectedTool(tool);
    mMapScene->enableSelectedTool();
}

void TileCollisionDock::applyChanges()
{
    if (mSynchronizing)
        return;

    ObjectGroup *objectGroup = static_cast<ObjectGroup*>(mDummyMapDocument->map()->layerAt(1));
    std::unique_ptr<ObjectGroup> clonedGroup;
    if (!objectGroup->isEmpty())
        clonedGroup.reset(objectGroup->clone());

    QUndoStack *undoStack = mTilesetDocument->undoStack();
    mApplyingChanges = true;
    undoStack->push(new ChangeTileObjectGroup(mTilesetDocument, mTile, std::move(clonedGroup)));
    mApplyingChanges = false;
}

void TileCollisionDock::tileObjectGroupChanged(Tile *tile)
{
    if (mTile != tile)
        return;
    if (mApplyingChanges)
        return;

    mSynchronizing = true;

    mDummyMapDocument->undoStack()->clear();
    auto selectedTool = mToolManager->selectedTool();

    LayerModel *layerModel = mDummyMapDocument->layerModel();
    delete layerModel->takeLayerAt(nullptr, 1);

    ObjectGroup *objectGroup;
    if (tile->objectGroup())
        objectGroup = tile->objectGroup()->clone();
    else
        objectGroup = new ObjectGroup;

    objectGroup->setDrawOrder(ObjectGroup::IndexOrder);

    layerModel->insertLayer(nullptr, 1, objectGroup);
    mDummyMapDocument->setCurrentLayer(objectGroup);

    mToolManager->selectTool(selectedTool);

    mSynchronizing = false;
}

void TileCollisionDock::tilesetTileOffsetChanged(Tileset *tileset)
{
    if (!mDummyMapDocument)
        return;

    auto tileLayer = mDummyMapDocument->map()->layerAt(0);
    auto tileOffset = tileset->tileOffset();
    mDummyMapDocument->layerModel()->setLayerOffset(tileLayer, -tileOffset);
}

void TileCollisionDock::cut()
{
    if (!mTile)
        return;

    copy();
    delete_(Cut);
}

void TileCollisionDock::copy()
{
    if (!mDummyMapDocument)
        return;

    ClipboardManager::instance()->copySelection(*mDummyMapDocument);
}

void TileCollisionDock::paste()
{
    paste(ClipboardManager::PasteDefault);
}

void TileCollisionDock::pasteInPlace()
{
    paste(ClipboardManager::PasteInPlace);
}

void TileCollisionDock::paste(ClipboardManager::PasteFlags flags)
{
    if (!mTile)
        return;

    ClipboardManager *clipboardManager = ClipboardManager::instance();
    const std::unique_ptr<Map> map(clipboardManager->map());
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

void TileCollisionDock::delete_(Operation operation)
{
    if (!mDummyMapDocument)
        return;

    const QList<MapObject*> &selectedObjects = mDummyMapDocument->selectedObjects();
    if (selectedObjects.isEmpty())
        return;

    auto command = new RemoveMapObjects(mDummyMapDocument.data(), selectedObjects);
    command->setText(operation == Delete ? tr("Delete") : tr("Cut"));

    mDummyMapDocument->undoStack()->push(command);
}

void TileCollisionDock::selectedObjectsChanged()
{
    setHasSelectedObjects(!mDummyMapDocument->selectedObjects().isEmpty());
    mActionMoveUp->setEnabled(hasSelectedObjects());
    mActionMoveDown->setEnabled(hasSelectedObjects());
}

void TileCollisionDock::setHasSelectedObjects(bool hasSelectedObjects)
{
    if (mHasSelectedObjects != hasSelectedObjects) {
        mHasSelectedObjects = hasSelectedObjects;
        emit hasSelectedObjectsChanged();
    }
}

void TileCollisionDock::selectAll()
{
    if (!mDummyMapDocument)
        return;

    ObjectGroup *objectGroup = static_cast<ObjectGroup*>(mDummyMapDocument->map()->layerAt(1));
    mDummyMapDocument->setSelectedObjects(objectGroup->objects());
}

void TileCollisionDock::moveObjectsUp()
{
    if (mDummyMapDocument)
        mDummyMapDocument->moveObjectsUp(mDummyMapDocument->selectedObjects());
}

void TileCollisionDock::moveObjectsDown()
{
    if (mDummyMapDocument)
        mDummyMapDocument->moveObjectsDown(mDummyMapDocument->selectedObjects());
}

void TileCollisionDock::changeEvent(QEvent *e)
{
    QDockWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        retranslateUi();
        break;
    default:
        break;
    }
}

void TileCollisionDock::retranslateUi()
{
    setWindowTitle(QCoreApplication::translate("Tiled::MainWindow", "Tile Collision Editor"));

    mActionMoveUp->setToolTip(tr("Move Objects Up"));
    mActionMoveDown->setToolTip(tr("Move Objects Down"));
}

} // namespace Tiled
