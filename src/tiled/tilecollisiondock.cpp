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
#include "editpolygontool.h"
#include "changetileobjectgroup.h"
#include "createobjecttool.h"
#include "createrectangleobjecttool.h"
#include "createellipseobjecttool.h"
#include "createpolygonobjecttool.h"
#include "createtemplatetool.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"
#include "mapobject.h"
#include "mapscene.h"
#include "mapview.h"
#include "objectgroup.h"
#include "objectselectiontool.h"
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
#include <QStatusBar>
#include <QToolBar>
#include <QUndoStack>
#include <QVBoxLayout>

namespace Tiled {
namespace Internal {

TileCollisionDock::TileCollisionDock(QWidget *parent)
    : QDockWidget(parent)
    , mTile(nullptr)
    , mTilesetDocument(nullptr)
    , mMapScene(new MapScene(this))
    , mMapView(new MapView(this, MapView::NoStaticContents))
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

    CreateObjectTool *rectangleObjectsTool = new CreateRectangleObjectTool(this);
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
    toolBar->addAction(mToolManager->registerTool(ellipseObjectsTool));
    toolBar->addAction(mToolManager->registerTool(polygonObjectsTool));
    toolBar->addAction(mToolManager->registerTool(templatesTool));

    auto widget = new QWidget(this);
    auto vertical = new QVBoxLayout(widget);
    vertical->setSpacing(0);
    vertical->setMargin(0);

    auto horizontal = new QHBoxLayout();
    horizontal->addWidget(toolBar, 1);

    vertical->addLayout(horizontal);
    vertical->addWidget(mMapView);

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
        mToolManager->setMapDocument(mDummyMapDocument.data());

        mMapScene->enableSelectedTool();

        connect(mDummyMapDocument->undoStack(), &QUndoStack::indexChanged,
                this, &TileCollisionDock::applyChanges);

        connect(mDummyMapDocument.data(), &MapDocument::selectedObjectsChanged,
                this, &TileCollisionDock::selectedObjectsChanged);

    } else {
        mDummyMapDocument.clear();
        mMapScene->setMapDocument(nullptr);
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

    const QList<MapObject*> selectedObjects = mDummyMapDocument->selectedObjects();
    if (selectedObjects.isEmpty())
        return;

    QUndoStack *undoStack = mDummyMapDocument->undoStack();
    undoStack->beginMacro(operation == Delete ? tr("Delete") : tr("Cut"));

    for (MapObject *mapObject : selectedObjects)
        undoStack->push(new RemoveMapObject(mDummyMapDocument.data(), mapObject));

    undoStack->endMacro();
}

void TileCollisionDock::selectedObjectsChanged()
{
    setHasSelectedObjects(!mDummyMapDocument->selectedObjects().isEmpty());
}

void TileCollisionDock::setHasSelectedObjects(bool hasSelectedObjects)
{
    if (mHasSelectedObjects != hasSelectedObjects) {
        mHasSelectedObjects = hasSelectedObjects;
        emit hasSelectedObjectsChanged();
    }
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
    setWindowTitle(QCoreApplication::translate("Tiled::Internal::MainWindow", "Tile Collision Editor"));
}

} // namespace Internal
} // namespace Tiled
