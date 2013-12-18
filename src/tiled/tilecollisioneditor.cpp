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

#include "editpolygontool.h"
#include "changetileobjectgroup.h"
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
#include "toolmanager.h"
#include "zoomable.h"

#include <QComboBox>
#include <QToolBar>
#include <QUndoStack>
#include <QVBoxLayout>

using namespace Tiled;
using namespace Tiled::Internal;

TileCollisionEditor::TileCollisionEditor(QWidget *parent)
    : QDockWidget(parent)
    , mTile(0)
    , mMapDocument(0)
    , mMapScene(new MapScene(this))
    , mToolManager(new ToolManager(this))
    , mApplyingChanges(false)
    , mSynchronizing(false)
{
    setObjectName(QLatin1String("tileCollisionEditor"));

    QWidget *widget = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setSpacing(0);
    layout->setMargin(5);

    MapView *mapView = new MapView(this);

    mapView->setScene(mMapScene);

    mapView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mapView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    CreateObjectTool *rectangleObjectsTool = new CreateObjectTool(
            CreateObjectTool::CreateRectangle, this);
    CreateObjectTool *ellipseObjectsTool = new CreateObjectTool(
            CreateObjectTool::CreateEllipse, this);
    CreateObjectTool *polygonObjectsTool = new CreateObjectTool(
            CreateObjectTool::CreatePolygon, this);
    CreateObjectTool *polylineObjectsTool = new CreateObjectTool(
            CreateObjectTool::CreatePolyline, this);

    mToolManager->registerTool(new ObjectSelectionTool(this));
    mToolManager->registerTool(new EditPolygonTool(this));
    mToolManager->registerTool(rectangleObjectsTool);
    mToolManager->registerTool(ellipseObjectsTool);
    mToolManager->registerTool(polygonObjectsTool);
    mToolManager->registerTool(polylineObjectsTool);

    QToolBar *toolBar = mToolManager->toolBar();
    toolBar->setMovable(false);
    toolBar->setFloatable(false);
    toolBar->setIconSize(QSize(16, 16));

    layout->addWidget(mapView);

    QHBoxLayout *horizontal = new QHBoxLayout;
    horizontal->setSpacing(0);
    horizontal->addWidget(toolBar, 1);

    layout->addLayout(horizontal);

    mMapScene->setSelectedTool(mToolManager->selectedTool());
    connect(mToolManager, SIGNAL(selectedToolChanged(AbstractTool*)),
            SLOT(setSelectedTool(AbstractTool*)));

    QComboBox *zoomComboBox = new QComboBox;
    horizontal->addWidget(zoomComboBox);

    Zoomable *zoomable = mapView->zoomable();
    zoomable->connectToComboBox(zoomComboBox);

    setWidget(widget);
    retranslateUi();
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

void TileCollisionEditor::setTile(Tile *tile)
{
    if (mTile == tile)
        return;

    mTile = tile;

    mMapScene->disableSelectedTool();
    MapDocument *previousDocument = mMapScene->mapDocument();

    if (tile) {
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
        mMapScene->setMapDocument(0);
        mToolManager->setMapDocument(0);
    }

    if (previousDocument) {
        previousDocument->undoStack()->disconnect(this);
        delete previousDocument;
    }
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
    ObjectGroup *objectGroup = static_cast<ObjectGroup*>(dummyDocument->map()->layerAt(1)->clone());

    QUndoStack *undoStack = mMapDocument->undoStack();
    mApplyingChanges = true;
    undoStack->push(new ChangeTileObjectGroup(mMapDocument, mTile, objectGroup));
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

void TileCollisionEditor::retranslateUi()
{
    setWindowTitle(tr("Collision Editor"));
}
