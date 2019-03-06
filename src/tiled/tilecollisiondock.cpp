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
#include "preferences.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "tilesetdocument.h"
#include "toolmanager.h"
#include "utils.h"
#include "zoomable.h"

#include <QActionGroup>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QMenu>
#include <QSettings>
#include <QShortcut>
#include <QSplitter>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QUndoStack>
#include <QVBoxLayout>

namespace Tiled {

static const char OBJECTS_VIEW_VISIBILITY[] = "TileCollisionDock/ObjectsViewVisibility";
static const char COLLISION_DOCK_SPLITTER_STATE[] = "TileCollisionDock/SplitterState";

TileCollisionDock::TileCollisionDock(QWidget *parent)
    : QDockWidget(parent)
    , mMapScene(new MapScene(this))
    , mMapView(new MapView(this, MapView::NoStaticContents))
    , mObjectsView(new ObjectsView(this))
    , mToolManager(new ToolManager(this))
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

    QToolBar *toolsToolBar = new QToolBar(this);
    toolsToolBar->setObjectName(QLatin1String("TileCollisionDockToolBar"));
    toolsToolBar->setMovable(false);
    toolsToolBar->setFloatable(false);
    toolsToolBar->setContextMenuPolicy(Qt::ActionsContextMenu);

    mToolManager = new ToolManager(this);
    toolsToolBar->addAction(mToolManager->registerTool(new ObjectSelectionTool(this)));
    toolsToolBar->addAction(mToolManager->registerTool(new EditPolygonTool(this)));
    toolsToolBar->addAction(mToolManager->registerTool(rectangleObjectsTool));
    toolsToolBar->addAction(mToolManager->registerTool(pointObjectsTool));
    toolsToolBar->addAction(mToolManager->registerTool(ellipseObjectsTool));
    toolsToolBar->addAction(mToolManager->registerTool(polygonObjectsTool));
    toolsToolBar->addAction(mToolManager->registerTool(templatesTool));

    mActionDuplicateObjects = new QAction(this);
    mActionDuplicateObjects->setIcon(QIcon(QLatin1String(":/images/16x16/stock-duplicate-16.png")));
    mActionRemoveObjects = new QAction(this);
    mActionRemoveObjects->setIcon(QIcon(QLatin1String(":/images/16x16/edit-delete.png")));
    mActionMoveUp = new QAction(this);
    mActionMoveUp->setIcon(QIcon(QLatin1String(":/images/16x16/go-up.png")));
    mActionMoveDown = new QAction(this);
    mActionMoveDown->setIcon(QIcon(QLatin1String(":/images/16x16/go-down.png")));
    mActionObjectProperties = new QAction(this);
    mActionObjectProperties->setIcon(QIcon(QLatin1String(":/images/16x16/document-properties.png")));

    Utils::setThemeIcon(mActionRemoveObjects, "edit-delete");
    Utils::setThemeIcon(mActionMoveUp, "go-up");
    Utils::setThemeIcon(mActionMoveDown, "go-down");
    Utils::setThemeIcon(mActionObjectProperties, "document-properties");

    QToolBar *objectsToolBar = new QToolBar(this);
    objectsToolBar->setMovable(false);
    objectsToolBar->setFloatable(false);
    objectsToolBar->setIconSize(Utils::smallIconSize());
    objectsToolBar->addAction(mActionDuplicateObjects);
    objectsToolBar->addAction(mActionRemoveObjects);
    objectsToolBar->addAction(mActionMoveUp);
    objectsToolBar->addAction(mActionMoveDown);
    objectsToolBar->addAction(mActionObjectProperties);

    mObjectsWidget = new QWidget;
    mObjectsWidget->setVisible(false);
    auto objectsVertical = new QVBoxLayout(mObjectsWidget);
    objectsVertical->setSpacing(0);
    objectsVertical->setMargin(0);
    objectsVertical->addWidget(mObjectsView);
    objectsVertical->addWidget(objectsToolBar);

    mObjectsViewSplitter = new QSplitter;
    mObjectsViewSplitter->addWidget(mMapView);
    mObjectsViewSplitter->addWidget(mObjectsWidget);

    mMapScene->setSelectedTool(mToolManager->selectedTool());
    connect(mToolManager, &ToolManager::selectedToolChanged,
            this, &TileCollisionDock::setSelectedTool);
    connect(mToolManager, &ToolManager::statusInfoChanged,
            this, &TileCollisionDock::statusInfoChanged);

    auto objectsViewActionGroup = new QActionGroup(this);

    mObjectsViewHiddenAction = new QAction(tr("Hidden"), objectsViewActionGroup);
    mObjectsViewHiddenAction->setData(QVariant::fromValue(Hidden));
    mObjectsViewHiddenAction->setCheckable(true);
    mObjectsViewHiddenAction->setChecked(true);

    mObjectsViewShowRightAction = new QAction(tr("Show Right"), objectsViewActionGroup);
    mObjectsViewShowRightAction->setData(QVariant::fromValue(ShowRight));
    mObjectsViewShowRightAction->setCheckable(true);

    mObjectsViewShowBottomAction = new QAction(tr("Show Bottom"), objectsViewActionGroup);
    mObjectsViewShowBottomAction->setData(QVariant::fromValue(ShowBottom));
    mObjectsViewShowBottomAction->setCheckable(true);

    connect(objectsViewActionGroup, &QActionGroup::triggered, this, [this] (QAction *action) {
        setObjectsViewVisibility(action->data().value<ObjectsViewVisibility>());
    });

    auto objectsViewMenu = new QMenu(this);
    objectsViewMenu->addActions(objectsViewActionGroup->actions());

    QIcon objectsViewIcon(QLatin1String("://images/16x16/layer-object.png"));
    objectsViewIcon.addFile(QLatin1String("://images/32x32/layer-object.png"));

    auto objectsViewButton = new QToolButton;
    objectsViewButton->setMenu(objectsViewMenu);
    objectsViewButton->setPopupMode(QToolButton::InstantPopup);
    objectsViewButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    objectsViewButton->setAutoRaise(true);
    objectsViewButton->setIcon(objectsViewIcon);
    objectsViewButton->setToolTip(tr("Objects list"));

    QComboBox *zoomComboBox = new QComboBox;
    mMapView->zoomable()->setComboBox(zoomComboBox);

    auto rightToolBar = new QToolBar;
    rightToolBar->setIconSize(Utils::smallIconSize());
    rightToolBar->addWidget(objectsViewButton);
    rightToolBar->addSeparator();
    rightToolBar->addWidget(zoomComboBox);

    auto horizontal = new QHBoxLayout;
    horizontal->setSpacing(qRound(Utils::dpiScaled(5)));
    horizontal->addWidget(toolsToolBar, 1);
    horizontal->addWidget(rightToolBar);

    auto widget = new QWidget(this);
    auto vertical = new QVBoxLayout(widget);
    vertical->setSpacing(0);
    vertical->setMargin(0);
    vertical->addLayout(horizontal);
    vertical->addWidget(mObjectsViewSplitter);

    auto selectAllShortcut = new QShortcut(Qt::CTRL + Qt::Key_A, mMapView, nullptr, nullptr, Qt::WidgetShortcut);
    connect(selectAllShortcut, &QShortcut::activated, this, &TileCollisionDock::selectAll);

    connect(mActionDuplicateObjects, &QAction::triggered, this, &TileCollisionDock::duplicateObjects);
    connect(mActionRemoveObjects, &QAction::triggered, this, &TileCollisionDock::removeObjects);
    connect(mActionMoveUp, &QAction::triggered, this, &TileCollisionDock::moveObjectsUp);
    connect(mActionMoveDown, &QAction::triggered, this, &TileCollisionDock::moveObjectsDown);
    connect(mActionObjectProperties, &QAction::triggered, this, &TileCollisionDock::objectProperties);

    retranslateUi();
    selectedObjectsChanged();   // disables actions

    setWidget(widget);
}

TileCollisionDock::~TileCollisionDock()
{
    setTile(nullptr);
}

void TileCollisionDock::saveState()
{
    QSettings *settings = Preferences::instance()->settings();
    settings->setValue(QLatin1String(OBJECTS_VIEW_VISIBILITY), QVariant::fromValue(mObjectsViewVisibility).toString());
    settings->setValue(QLatin1String(COLLISION_DOCK_SPLITTER_STATE), mObjectsViewSplitter->saveState());
}

void TileCollisionDock::restoreState()
{
    const QSettings *settings = Preferences::instance()->settings();
    setObjectsViewVisibility(settings->value(QLatin1String(OBJECTS_VIEW_VISIBILITY), Hidden).value<ObjectsViewVisibility>());
    mObjectsViewSplitter->restoreState(settings->value(QLatin1String(COLLISION_DOCK_SPLITTER_STATE)).toByteArray());
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

        mDummyMapDocument = MapDocumentPtr::create(std::move(map));
        mDummyMapDocument->setAllowHidingObjects(false);
        mDummyMapDocument->setAllowTileObjects(false);
        mDummyMapDocument->setCurrentLayer(objectGroup);
        mDummyMapDocument->setSelectedLayers({objectGroup});

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
    mDummyMapDocument->setSelectedLayers({objectGroup});
    mObjectsView->setRootIndex(mObjectsView->layerViewIndex(objectGroup));

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
    setHasSelectedObjects(mDummyMapDocument ? !mDummyMapDocument->selectedObjects().isEmpty() : false);

    mActionDuplicateObjects->setEnabled(hasSelectedObjects());
    mActionRemoveObjects->setEnabled(hasSelectedObjects());
    mActionMoveUp->setEnabled(hasSelectedObjects());
    mActionMoveDown->setEnabled(hasSelectedObjects());
    mActionObjectProperties->setEnabled(hasSelectedObjects());
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

void TileCollisionDock::duplicateObjects()
{
    if (mDummyMapDocument)
        mDummyMapDocument->duplicateObjects(mDummyMapDocument->selectedObjects());
}

void TileCollisionDock::removeObjects()
{
    if (mDummyMapDocument)
        mDummyMapDocument->removeObjects(mDummyMapDocument->selectedObjects());
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

void TileCollisionDock::objectProperties()
{
    if (!mDummyMapDocument)
        return;

    const auto &selectedObjects = mDummyMapDocument->selectedObjects();
    mDummyMapDocument->setCurrentObject(selectedObjects.first());
    emit mDummyMapDocument->editCurrentObject();
}

void TileCollisionDock::setObjectsViewVisibility(ObjectsViewVisibility visibility)
{
    if (mObjectsViewVisibility == visibility)
        return;

    mObjectsViewVisibility = visibility;

    switch (visibility) {
    case Hidden:
        mObjectsWidget->setVisible(false);
        mObjectsViewHiddenAction->setChecked(true);
        break;
    case ShowRight:
        mObjectsWidget->setVisible(true);
        mObjectsViewSplitter->setOrientation(Qt::Horizontal);
        mObjectsViewShowRightAction->setChecked(true);
        break;
    case ShowBottom:
        mObjectsWidget->setVisible(true);
        mObjectsViewSplitter->setOrientation(Qt::Vertical);
        mObjectsViewShowBottomAction->setChecked(true);
        break;
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
    setWindowTitle(QCoreApplication::translate("Tiled::MainWindow", "Tile Collision Editor"));

    mActionDuplicateObjects->setText(tr("Duplicate Objects"));
    mActionRemoveObjects->setText(tr("Remove Objects"));
    mActionMoveUp->setToolTip(tr("Move Objects Up"));
    mActionMoveDown->setToolTip(tr("Move Objects Down"));
    mActionObjectProperties->setToolTip(tr("Object Properties"));
}

} // namespace Tiled
