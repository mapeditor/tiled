/*
 * mapeditor.cpp
 * Copyright 2016, Thorbjørn Lindeijer <bjorn@lindijer.nl>
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

#include "mapeditor.h"

#include "addremovetileset.h"
#include "brokenlinks.h"
#include "bucketfilltool.h"
#include "createellipseobjecttool.h"
#include "createobjecttool.h"
#include "createpointobjecttool.h"
#include "createpolygonobjecttool.h"
#include "createpolylineobjecttool.h"
#include "createrectangleobjecttool.h"
#include "createtemplatetool.h"
#include "createtextobjecttool.h"
#include "createtileobjecttool.h"
#include "documentmanager.h"
#include "editpolygontool.h"
#include "eraser.h"
#include "filechangedwarning.h"
#include "layerdock.h"
#include "layermodel.h"
#include "layeroffsettool.h"
#include "magicwandtool.h"
#include "maintoolbar.h"
#include "mapdocumentactionhandler.h"
#include "mapscene.h"
#include "mapsdock.h"
#include "mapview.h"
#include "minimapdock.h"
#include "newtilesetdialog.h"
#include "objectsdock.h"
#include "objecttemplate.h"
#include "templatesdock.h"
#include "objectselectiontool.h"
#include "painttilelayer.h"
#include "preferences.h"
#include "propertiesdock.h"
#include "reversingproxymodel.h"
#include "selectsametiletool.h"
#include "shapefilltool.h"
#include "stampbrush.h"
#include "terrain.h"
#include "terrainbrush.h"
#include "terraindock.h"
#include "tile.h"
#include "tileselectiontool.h"
#include "tilesetdock.h"
#include "tilesetdocument.h"
#include "tilesetformat.h"
#include "tilesetmanager.h"
#include "tilestamp.h"
#include "tilestampmanager.h"
#include "tilestampsdock.h"
#include "toolmanager.h"
#include "treeviewcombobox.h"
#include "undodock.h"
#include "wangbrush.h"
#include "wangdock.h"
#include "wangset.h"
#include "zoomable.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QIdentityProxyModel>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>
#include <QShortcut>
#include <QSignalMapper>
#include <QStackedWidget>
#include <QStatusBar>
#include <QToolBar>

static const char SIZE_KEY[] = "MapEditor/Size";
static const char STATE_KEY[] = "MapEditor/State";
static const char MAPSTATES_KEY[] = "MapEditor/MapStates";

namespace Tiled {
namespace Internal {

/**
 * A proxy model that makes sure no items are checked or checkable and that
 * there is only one column.
 *
 * Used in the layer combo box, since the checkboxes can't be used in that
 * context but are otherwise anyway rendered there on Windows.
 */
class ComboBoxProxyModel : public QIdentityProxyModel
{
public:
    explicit ComboBoxProxyModel(QObject *parent = nullptr)
        : QIdentityProxyModel(parent)
    {}

    int columnCount(const QModelIndex &) const override { return 1; }
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
};

QVariant ComboBoxProxyModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::CheckStateRole)
        return QVariant();

    return QIdentityProxyModel::data(index, role);
}

Qt::ItemFlags ComboBoxProxyModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags rc = QIdentityProxyModel::flags(index);
    return rc & ~Qt::ItemIsUserCheckable;
}



MapEditor::MapEditor(QObject *parent)
    : Editor(parent)
    , mMainWindow(new QMainWindow)
    , mLayerDock(new LayerDock(mMainWindow))
    , mWidgetStack(new QStackedWidget(mMainWindow))
    , mCurrentMapDocument(nullptr)
    , mMapsDock(new MapsDock(mMainWindow))
    , mUndoDock(new UndoDock(mMainWindow))
    , mObjectsDock(new ObjectsDock(mMainWindow))
    , mTemplatesDock(new TemplatesDock(mMainWindow))
    , mTilesetDock(new TilesetDock(mMainWindow))
    , mTerrainDock(new TerrainDock(mMainWindow))
    , mWangDock(new WangDock(mMainWindow))
    , mMiniMapDock(new MiniMapDock(mMainWindow))
    , mLayerComboBox(new TreeViewComboBox)
    , mComboBoxProxyModel(new ComboBoxProxyModel(this))
    , mReversingProxyModel(new ReversingProxyModel(this))
    , mZoomable(nullptr)
    , mZoomComboBox(new QComboBox)
    , mStatusInfoLabel(new QLabel)
    , mMainToolBar(new MainToolBar(mMainWindow))
    , mToolManager(new ToolManager(this))
    , mSelectedTool(nullptr)
    , mViewWithTool(nullptr)
    , mTileStampManager(new TileStampManager(*mToolManager, this))
{
    mMainWindow->setDockOptions(mMainWindow->dockOptions() | QMainWindow::GroupedDragging);
    mMainWindow->setDockNestingEnabled(true);
    mMainWindow->setCentralWidget(mWidgetStack);

    mToolsToolBar = new QToolBar(mMainWindow);
    mToolsToolBar->setObjectName(QLatin1String("toolsToolBar"));

    mToolSpecificToolBar = new QToolBar(mMainWindow);
    mToolSpecificToolBar->setObjectName(QLatin1String("toolSpecificToolBar"));

    mStampBrush = new StampBrush(this);
    mTerrainBrush = new TerrainBrush(this);
    mWangBrush = new WangBrush(this);
    mBucketFillTool = new BucketFillTool(this);
    mEditPolygonTool = new EditPolygonTool(this);
    mShapeFillTool = new ShapeFillTool(this);
    CreateObjectTool *tileObjectsTool = new CreateTileObjectTool(this);
    CreateTemplateTool *templatesTool = new CreateTemplateTool(this);
    CreateObjectTool *rectangleObjectsTool = new CreateRectangleObjectTool(this);
    CreateObjectTool *pointObjectsTool = new CreatePointObjectTool(this);
    CreateObjectTool *ellipseObjectsTool = new CreateEllipseObjectTool(this);
    CreateObjectTool *polygonObjectsTool = new CreatePolygonObjectTool(this);
    CreateObjectTool *polylineObjectsTool = new CreatePolylineObjectTool(this);
    CreateObjectTool *textObjectsTool = new CreateTextObjectTool(this);

    mToolsToolBar->addAction(mToolManager->registerTool(mStampBrush));
    mToolsToolBar->addAction(mToolManager->registerTool(mTerrainBrush));
    mToolsToolBar->addAction(mToolManager->registerTool(mWangBrush));
    mToolsToolBar->addAction(mToolManager->registerTool(mBucketFillTool));
    mToolsToolBar->addAction(mToolManager->registerTool(mShapeFillTool));
    mToolsToolBar->addAction(mToolManager->registerTool(new Eraser(this)));
    mToolsToolBar->addAction(mToolManager->registerTool(new TileSelectionTool(this)));
    mToolsToolBar->addAction(mToolManager->registerTool(new MagicWandTool(this)));
    mToolsToolBar->addAction(mToolManager->registerTool(new SelectSameTileTool(this)));
    mToolsToolBar->addSeparator();
    mToolsToolBar->addAction(mToolManager->registerTool(new ObjectSelectionTool(this)));
    mToolsToolBar->addAction(mToolManager->registerTool(mEditPolygonTool));
    mToolsToolBar->addAction(mToolManager->registerTool(rectangleObjectsTool));
    mToolsToolBar->addAction(mToolManager->registerTool(pointObjectsTool));
    mToolsToolBar->addAction(mToolManager->registerTool(ellipseObjectsTool));
    mToolsToolBar->addAction(mToolManager->registerTool(polygonObjectsTool));
    mToolsToolBar->addAction(mToolManager->registerTool(polylineObjectsTool));
    mToolsToolBar->addAction(mToolManager->registerTool(tileObjectsTool));
    mToolsToolBar->addAction(mToolManager->registerTool(templatesTool));
    mToolsToolBar->addAction(mToolManager->registerTool(textObjectsTool));
    mToolsToolBar->addSeparator();
    mToolsToolBar->addAction(mToolManager->registerTool(new LayerOffsetTool(this)));

    mToolManager->createShortcuts(mMainWindow);

    mMainWindow->addToolBar(mMainToolBar);
    mMainWindow->addToolBar(mToolsToolBar);
    mMainWindow->addToolBar(mToolSpecificToolBar);

    mPropertiesDock = new PropertiesDock(mMainWindow);
    mTemplatesDock->setPropertiesDock(mPropertiesDock);
    mTileStampsDock = new TileStampsDock(mTileStampManager, mMainWindow);

    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mLayerDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mPropertiesDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mMapsDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mUndoDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mObjectsDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mTemplatesDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mMiniMapDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mTerrainDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mWangDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mTilesetDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mTileStampsDock);

    mMainWindow->tabifyDockWidget(mUndoDock, mMapsDock);
    mMainWindow->tabifyDockWidget(mTemplatesDock, mTileStampsDock);
    mMainWindow->tabifyDockWidget(mMiniMapDock, mObjectsDock);
    mMainWindow->tabifyDockWidget(mObjectsDock, mLayerDock);
    mMainWindow->tabifyDockWidget(mTerrainDock, mWangDock);
    mMainWindow->tabifyDockWidget(mWangDock, mTilesetDock);

    // These dock widgets may not be immediately useful to many people, so
    // they are hidden by default.
    mMapsDock->setVisible(false);
    mUndoDock->setVisible(false);
    mTemplatesDock->setVisible(false);
    mWangDock->setVisible(false);
    mTileStampsDock->setVisible(false);

    mComboBoxProxyModel->setSourceModel(mReversingProxyModel);
    mLayerComboBox->setModel(mComboBoxProxyModel);
    mLayerComboBox->setMinimumContentsLength(10);
    mLayerComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(mLayerComboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this, &MapEditor::layerComboActivated);

    mMainWindow->statusBar()->addPermanentWidget(mLayerComboBox);
    mMainWindow->statusBar()->addPermanentWidget(mZoomComboBox);
    mMainWindow->statusBar()->addWidget(mStatusInfoLabel);

    connect(mWidgetStack, &QStackedWidget::currentChanged, this, &MapEditor::currentWidgetChanged);
    connect(mToolManager, &ToolManager::statusInfoChanged, this, &MapEditor::updateStatusInfoLabel);
    connect(mTilesetDock, &TilesetDock::currentTileChanged, mToolManager, &ToolManager::setTile);
    connect(mTilesetDock, &TilesetDock::currentTileChanged, mTemplatesDock, &TemplatesDock::setTile);
    connect(mTilesetDock, &TilesetDock::stampCaptured, this, &MapEditor::setStamp);
    connect(mTilesetDock, &TilesetDock::localFilesDropped, this, &MapEditor::filesDroppedOnTilesetDock);
    connect(mTemplatesDock, &TemplatesDock::currentTemplateChanged, mToolManager, &ToolManager::setObjectTemplate);

    connect(mStampBrush, &StampBrush::stampChanged, this, &MapEditor::setStamp);
    connect(mBucketFillTool, &BucketFillTool::stampChanged, this, &MapEditor::setStamp);
    connect(mShapeFillTool, &ShapeFillTool::stampChanged, this, &MapEditor::setStamp);
    connect(mStampBrush, &StampBrush::randomChanged, this, &MapEditor::setRandom);
    connect(mBucketFillTool, &BucketFillTool::randomChanged, this, &MapEditor::setRandom);
    connect(mShapeFillTool, &ShapeFillTool::randomChanged, this, &MapEditor::setRandom);
    connect(mStampBrush, &StampBrush::wangFillChanged, this, &MapEditor::setWangFill);
    connect(mBucketFillTool, &BucketFillTool::wangFillChanged, this, &MapEditor::setWangFill);
    connect(mShapeFillTool, &ShapeFillTool::wangFillChanged, this, &MapEditor::setWangFill);

    connect(mTerrainDock, &TerrainDock::currentTerrainChanged,
            mTerrainBrush, &TerrainBrush::setTerrain);
    connect(mTerrainDock, &TerrainDock::selectTerrainBrush,
            this, &MapEditor::selectTerrainBrush);
    connect(mTerrainBrush, &TerrainBrush::terrainCaptured,
            mTerrainDock, &TerrainDock::setCurrentTerrain);

    connect(mWangDock, &WangDock::currentWangSetChanged,
            mBucketFillTool, &BucketFillTool::setWangSet);
    connect(mWangDock, &WangDock::currentWangSetChanged,
            mShapeFillTool, &ShapeFillTool::setWangSet);
    connect(mWangDock, &WangDock::currentWangSetChanged,
            mStampBrush, &StampBrush::setWangSet);
    connect(mWangDock, &WangDock::currentWangSetChanged,
            mWangBrush, &WangBrush::wangSetChanged);
    connect(mWangDock, &WangDock::selectWangBrush,
            this, &MapEditor::selectWangBrush);
    connect(mWangDock, &WangDock::wangColorChanged,
            mWangBrush, &WangBrush::wangColorChanged);
    connect(mWangBrush, &WangBrush::colorCaptured,
            mWangDock, &WangDock::onColorCaptured);

    connect(mTileStampsDock, SIGNAL(setStamp(TileStamp)),
            this, SLOT(setStamp(TileStamp)));

    setSelectedTool(mToolManager->selectedTool());
    connect(mToolManager, &ToolManager::selectedToolChanged,
            this, &MapEditor::setSelectedTool);

    connect(mTemplatesDock, &TemplatesDock::templateEdited,
            this, &MapEditor::updateTemplateInstances);

    setupQuickStamps();
    retranslateUi();
    connect(Preferences::instance(), &Preferences::languageChanged, this, &MapEditor::retranslateUi);

    QSettings *settings = Preferences::instance()->settings();
    mMapStates = settings->value(QLatin1String(MAPSTATES_KEY)).toMap();
}

MapEditor::~MapEditor()
{
}

void MapEditor::saveState()
{
    QSettings *settings = Preferences::instance()->settings();
    settings->setValue(QLatin1String(SIZE_KEY), mMainWindow->size());
    settings->setValue(QLatin1String(STATE_KEY), mMainWindow->saveState());
}

void MapEditor::restoreState()
{
    QSettings *settings = Preferences::instance()->settings();
    QSize size = settings->value(QLatin1String(SIZE_KEY)).toSize();
    if (!size.isEmpty()) {
        mMainWindow->resize(size.width(), size.height());
        mMainWindow->restoreState(settings->value(QLatin1String(STATE_KEY)).toByteArray());
    }
}

void MapEditor::addDocument(Document *document)
{
    MapDocument *mapDocument = qobject_cast<MapDocument*>(document);
    Q_ASSERT(mapDocument);

    MapView *view = new MapView(mWidgetStack);
    MapScene *scene = new MapScene(view); // scene is owned by the view

    scene->setMapDocument(mapDocument);
    view->setScene(scene);

    mWidgetForMap.insert(mapDocument, view);
    mWidgetStack->addWidget(view);

    // restore the previous state for this map
    QVariantMap mapState = mMapStates.value(document->fileName()).toMap();
    if (!mapState.isEmpty()) {
        qreal scale = mapState.value(QLatin1String("scale")).toReal();
        if (scale > 0)
            view->zoomable()->setScale(scale);

        const QPointF viewCenter = mapState.value(QLatin1String("viewCenter")).toPointF();
        view->forceCenterOn(viewCenter);

        int layerIndex = mapState.value(QLatin1String("selectedLayer")).toInt();
        if (Layer *layer = layerAtGlobalIndex(mapDocument->map(), layerIndex))
            mapDocument->setCurrentLayer(layer);
    }
}

void MapEditor::removeDocument(Document *document)
{
    MapDocument *mapDocument = qobject_cast<MapDocument*>(document);
    Q_ASSERT(mapDocument);
    Q_ASSERT(mWidgetForMap.contains(mapDocument));

    MapView *mapView = mWidgetForMap.take(mapDocument);

    // remember the state of this map before deleting the view
    if (!mapDocument->fileName().isEmpty()) {
        QVariantMap mapState;
        mapState.insert(QLatin1String("scale"), mapView->zoomable()->scale());
        mapState.insert(QLatin1String("viewCenter"), mapView->mapToScene(mapView->viewport()->rect().center()));
        mapState.insert(QLatin1String("selectedLayer"), globalIndex(mapDocument->currentLayer()));
        mMapStates.insert(mapDocument->fileName(), mapState);

        Preferences *prefs = Preferences::instance();
        QSettings *settings = prefs->settings();
        settings->setValue(QLatin1String(MAPSTATES_KEY), mMapStates);
    }

    // remove first, to keep it valid while the current widget changes
    mWidgetStack->removeWidget(mapView);
    delete mapView;
}

void MapEditor::setCurrentDocument(Document *document)
{
    MapDocument *mapDocument = qobject_cast<MapDocument*>(document);
    Q_ASSERT(mapDocument || !document);

    if (mCurrentMapDocument == mapDocument)
        return;

    if (mCurrentMapDocument)
        mCurrentMapDocument->disconnect(this);

    mCurrentMapDocument = mapDocument;

    MapView *mapView = mWidgetForMap.value(mapDocument);
    if (mapView)
        mWidgetStack->setCurrentWidget(mapView);

    mLayerDock->setMapDocument(mapDocument);

    if (mZoomable) {
        mZoomable->setComboBox(nullptr);
        mZoomable = nullptr;
    }

    mPropertiesDock->setDocument(mapDocument);
    mUndoDock->setStack(document ? document->undoStack() : nullptr);
    mObjectsDock->setMapDocument(mapDocument);
    mTilesetDock->setMapDocument(mapDocument);
    mTerrainDock->setDocument(mapDocument);
    mWangDock->setDocument(mapDocument);
    mMiniMapDock->setMapDocument(mapDocument);

    if (mapDocument) {
        connect(mapDocument, &MapDocument::currentLayerChanged,
                this, &MapEditor::updateLayerComboIndex);
//        connect(mapDocument, SIGNAL(selectedAreaChanged(QRegion,QRegion)),
//                SLOT(updateActions()));
//        connect(mapDocument, SIGNAL(selectedObjectsChanged()),
//                SLOT(updateActions()));

        if (mapView) {
            mZoomable = mapView->zoomable();
            mZoomable->setComboBox(mZoomComboBox);
        }

        connect(mCurrentMapDocument, &MapDocument::currentObjectChanged,
                this, [this, mapDocument](){ mPropertiesDock->setDocument(mapDocument); });

        connect(mapView, &MapView::focused,
                this, [this, mapDocument](){ mPropertiesDock->setDocument(mapDocument); });

        mReversingProxyModel->setSourceModel(mapDocument->layerModel());
    } else {
        mReversingProxyModel->setSourceModel(nullptr);
    }

    mLayerComboBox->setEnabled(mapDocument);
    updateLayerComboIndex();

    // Take the currently active tool to the new map view
    if (mViewWithTool) {
        MapScene *mapScene = mViewWithTool->mapScene();
        mapScene->disableSelectedTool();
        mViewWithTool = nullptr;
    }

    mToolManager->setMapDocument(mapDocument);

    if (mapView) {
        MapScene *mapScene = mapView->mapScene();
        mapScene->setSelectedTool(mSelectedTool);
        mapScene->enableSelectedTool();
        if (mSelectedTool)
            mapView->viewport()->setCursor(mSelectedTool->cursor());
        else
            mapView->viewport()->unsetCursor();
        mViewWithTool = mapView;
    }
}

Document *MapEditor::currentDocument() const
{
    return mCurrentMapDocument;
}

QWidget *MapEditor::editorWidget() const
{
    return mMainWindow;
}

QList<QToolBar *> MapEditor::toolBars() const
{
    return QList<QToolBar*> {
        mMainToolBar,
        mToolsToolBar,
        mToolSpecificToolBar
    };
}

QList<QDockWidget *> MapEditor::dockWidgets() const
{
    return QList<QDockWidget*> {
        mPropertiesDock,
        mLayerDock,
        mMapsDock,
        mUndoDock,
        mObjectsDock,
        mTemplatesDock,
        mTilesetDock,
        mTerrainDock,
        mWangDock,
        mMiniMapDock,
        mTileStampsDock
    };
}

Editor::StandardActions MapEditor::enabledStandardActions() const
{
    StandardActions standardActions;

    if (mCurrentMapDocument) {
        Layer *currentLayer = mCurrentMapDocument->currentLayer();

        bool tileLayerSelected = currentLayer && currentLayer->isTileLayer();
        bool objectsSelected = !mCurrentMapDocument->selectedObjects().isEmpty();
        QRegion selection = mCurrentMapDocument->selectedArea();

        if ((tileLayerSelected && !selection.isEmpty()) || objectsSelected)
            standardActions |= CutAction | CopyAction | DeleteAction;

        if (ClipboardManager::instance()->hasMap())
            standardActions |= PasteAction | PasteInPlaceAction;
    }

    return standardActions;
}

void MapEditor::performStandardAction(StandardAction action)
{
    switch (action) {
    case CutAction:
        MapDocumentActionHandler::instance()->cut();
        break;
    case CopyAction:
        MapDocumentActionHandler::instance()->copy();
        break;
    case PasteAction:
        paste(ClipboardManager::PasteDefault);
        break;
    case PasteInPlaceAction:
        paste(ClipboardManager::PasteInPlace);
        break;
    case DeleteAction:
        if (mEditPolygonTool->hasSelectedHandles())
            mEditPolygonTool->deleteNodes();
        else
            MapDocumentActionHandler::instance()->delete_();
        break;
    }
}

Zoomable *MapEditor::zoomable() const
{
    if (auto view = currentMapView())
        return view->zoomable();
    return nullptr;
}

void MapEditor::showMessage(const QString &text, int timeout)
{
    mMainWindow->statusBar()->showMessage(text, timeout);
}

void MapEditor::setSelectedTool(AbstractTool *tool)
{
    if (mSelectedTool == tool)
        return;

    if (mSelectedTool) {
        disconnect(mSelectedTool, &AbstractTool::cursorChanged,
                   this, &MapEditor::cursorChanged);
    }

    mSelectedTool = tool;
    mToolSpecificToolBar->clear();

    if (mViewWithTool) {
        MapScene *mapScene = mViewWithTool->mapScene();
        mapScene->disableSelectedTool();

        if (tool) {
            mapScene->setSelectedTool(tool);
            mapScene->enableSelectedTool();
        }

        if (tool)
            mViewWithTool->viewport()->setCursor(tool->cursor());
        else
            mViewWithTool->viewport()->unsetCursor();
    }

    if (tool) {
        connect(tool, &AbstractTool::cursorChanged,
                this, &MapEditor::cursorChanged);

        tool->populateToolBar(mToolSpecificToolBar);
    }
}

void MapEditor::paste(ClipboardManager::PasteFlags flags)
{
    if (!mCurrentMapDocument)
        return;

    Layer *currentLayer = mCurrentMapDocument->currentLayer();
    if (!currentLayer)
        return;

    ClipboardManager *clipboardManager = ClipboardManager::instance();
    QScopedPointer<Map> map(clipboardManager->map());
    if (!map)
        return;

    // We can currently only handle maps with a single layer
    if (map->layerCount() != 1)
        return;

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->addReferences(map->tilesets());

    mCurrentMapDocument->unifyTilesets(map.data());
    Layer *layer = map->layerAt(0);

    if (layer->isTileLayer()) {
        if (flags & ClipboardManager::PasteInPlace) {
            TileLayer *source = static_cast<TileLayer*>(layer);
            TileLayer *target = currentLayer->asTileLayer();

            if (target) {
                // Paste onto the current layer, using the same position as where
                // the copied piece came from.
                auto undoStack = mCurrentMapDocument->undoStack();
                undoStack->push(new PaintTileLayer(mCurrentMapDocument,
                                                   target,
                                                   source->x(),
                                                   source->y(),
                                                   source));
            }
        } else {
            // Reset selection and paste into the stamp brush
            MapDocumentActionHandler::instance()->selectNone();
            layer->setPosition(0, 0);   // Make sure the tile layer is at origin
            Map *stamp = map.take();    // TileStamp will take ownership
            setStamp(TileStamp(stamp));
            tilesetManager->removeReferences(stamp->tilesets());
            mToolManager->selectTool(mStampBrush);
        }
    } else if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
        const MapView *view = currentMapView();
        clipboardManager->pasteObjectGroup(objectGroup, mCurrentMapDocument, view, flags);
    }

    if (map)
        tilesetManager->removeReferences(map->tilesets());
}

void MapEditor::flip(FlipDirection direction)
{
    if (mStampBrush->isEnabled()) {
        const TileStamp &stamp = mStampBrush->stamp();
        if (!stamp.isEmpty())
            setStamp(stamp.flipped(direction));

    } else if (mCurrentMapDocument) {
        mCurrentMapDocument->flipSelectedObjects(direction);
    }
}

void MapEditor::rotate(RotateDirection direction)
{
    if (mStampBrush->isEnabled()) {
        const TileStamp &stamp = mStampBrush->stamp();
        if (!stamp.isEmpty())
            setStamp(stamp.rotated(direction));

    } else if (mCurrentMapDocument) {
        mCurrentMapDocument->rotateSelectedObjects(direction);
    }
}

void MapEditor::setRandom(bool value)
{
    mStampBrush->setRandom(value);

    auto fillMethod = value ? AbstractTileFillTool::RandomFill :
                              AbstractTileFillTool::TileFill;

    mBucketFillTool->setFillMethod(fillMethod);
    mShapeFillTool->setFillMethod(fillMethod);
}

void MapEditor::setWangFill(bool value)
{
    mStampBrush->setWangFill(value);

    auto fillMethod = value ? AbstractTileFillTool::WangFill :
                              AbstractTileFillTool::TileFill;

    mBucketFillTool->setFillMethod(fillMethod);
    mShapeFillTool->setFillMethod(fillMethod);
}

/**
 * Sets the current stamp, which is used by both the stamp brush and the bucket
 * fill tool.
 */
void MapEditor::setStamp(const TileStamp &stamp)
{
    if (stamp.isEmpty())
        return;

    mStampBrush->setStamp(stamp);
    mBucketFillTool->setStamp(stamp);
    mShapeFillTool->setStamp(stamp);

    // When selecting a new stamp, it makes sense to switch to a stamp tool
    AbstractTool *selectedTool = mToolManager->selectedTool();
    if (selectedTool != mStampBrush
            && selectedTool != mBucketFillTool
            && selectedTool != mShapeFillTool)
        mToolManager->selectTool(mStampBrush);

    mTilesetDock->selectTilesInStamp(stamp);
}

void MapEditor::selectTerrainBrush()
{
    mToolManager->selectTool(mTerrainBrush);
}

void MapEditor::selectWangBrush()
{
    mToolManager->selectTool(mWangBrush);
}

void MapEditor::currentWidgetChanged()
{
    auto mapView = static_cast<MapView*>(mWidgetStack->currentWidget());
    setCurrentDocument(mapView ? mapView->mapScene()->mapDocument() : nullptr);
}

//void MapEditor::changeEvent(QEvent *event)
//{
//    QMainWindow::changeEvent(event);
//    switch (event->type()) {
//    case QEvent::LanguageChange:
//        mToolManager->retranslateTools();
//        retranslateUi();
//        break;
//    default:
//        break;
//    }
//}

void MapEditor::cursorChanged(const QCursor &cursor)
{
    if (mViewWithTool)
        mViewWithTool->viewport()->setCursor(cursor);
}

void MapEditor::updateStatusInfoLabel(const QString &statusInfo)
{
    mStatusInfoLabel->setText(statusInfo);
}

void MapEditor::layerComboActivated()
{
    if (!mCurrentMapDocument)
        return;

    const QModelIndex comboIndex = mLayerComboBox->currentModelIndex();
    const QModelIndex reversedIndex = mComboBoxProxyModel->mapToSource(comboIndex);
    const QModelIndex sourceIndex = mReversingProxyModel->mapToSource(reversedIndex);
    Layer *layer = mCurrentMapDocument->layerModel()->toLayer(sourceIndex);
    if (!layer)
        return;

    mCurrentMapDocument->setCurrentLayer(layer);
}

void MapEditor::updateLayerComboIndex()
{
    QModelIndex index;

    if (mCurrentMapDocument) {
        const auto currentLayer = mCurrentMapDocument->currentLayer();
        const QModelIndex sourceIndex = mCurrentMapDocument->layerModel()->index(currentLayer);
        const QModelIndex reversedIndex = mReversingProxyModel->mapFromSource(sourceIndex);
        index = mComboBoxProxyModel->mapFromSource(reversedIndex);
    }

    mLayerComboBox->setCurrentModelIndex(index);
}

void MapEditor::addExternalTilesets(const QStringList &fileNames)
{
    handleExternalTilesetsAndImages(fileNames, false);
}

void MapEditor::filesDroppedOnTilesetDock(const QStringList &fileNames)
{
    handleExternalTilesetsAndImages(fileNames, true);
}

void MapEditor::updateTemplateInstances(const ObjectTemplate *objectTemplate)
{
    QHashIterator<MapDocument*, MapView*> mapDocumentIterator(mWidgetForMap);
    while (mapDocumentIterator.hasNext()) {
        mapDocumentIterator.next();
        mapDocumentIterator.key()->updateTemplateInstances(objectTemplate);
    }
}

void MapEditor::handleExternalTilesetsAndImages(const QStringList &fileNames,
                                                bool handleImages)
{
    // These files could be either external tilesets, in which case we'll add
    // them to the current map, or images, in which case we'll offer to create
    // tilesets based on them (unless handleImages is false).

    QVector<SharedTileset> tilesets;

    for (const QString &fileName : fileNames) {
        QString error;

        // Check if the file is an already loaded tileset
        SharedTileset tileset = TilesetManager::instance()->findTileset(fileName);
        if (tileset) {
            tilesets.append(tileset);
            continue;
        }

        // Check if the file is has a supported tileset format
        TilesetFormat *tilesetFormat = findSupportingTilesetFormat(fileName);
        if (tilesetFormat) {
            tileset = tilesetFormat->read(fileName);
            if (tileset) {
                tileset->setFormat(tilesetFormat);
                tilesets.append(tileset);
                continue;
            } else {
                error = tilesetFormat->errorString();
            }
        }

        if (handleImages) {
            // Check if the file is a supported image format
            QImage image(fileName);
            if (!image.isNull()) {
                tileset = newTileset(fileName, image);
                if (tileset)
                    tilesets.append(tileset);
                continue;
            }
        }

        if (fileNames.size() == 1) {
            QMessageBox::critical(mMainWindow, tr("Error Reading Tileset"), error);
            return;
        } else {
            int result;

            result = QMessageBox::warning(mMainWindow, tr("Error Reading Tileset"),
                                          tr("%1: %2").arg(fileName, error),
                                          QMessageBox::Abort | QMessageBox::Ignore,
                                          QMessageBox::Ignore);

            if (result == QMessageBox::Abort)
                return;
        }
    }

    // Filter out any tilesets that are already referenced by the map
    auto it = tilesets.begin();
    auto end = std::remove_if(it, tilesets.end(), [this](SharedTileset &tileset) {
        return mCurrentMapDocument->map()->tilesets().contains(tileset);
    });

    if (it != end) {
        QUndoStack *undoStack = mCurrentMapDocument->undoStack();
        undoStack->beginMacro(tr("Add %n Tileset(s)", "", tilesets.size()));
        for (; it != end; ++it)
            undoStack->push(new AddTileset(mCurrentMapDocument, *it));
        undoStack->endMacro();
    }
}

SharedTileset MapEditor::newTileset(const QString &path, const QImage &image)
{
    NewTilesetDialog newTileset(mMainWindow->window());
    newTileset.setImagePath(path);

    SharedTileset tileset = newTileset.createTileset();
    if (!tileset)
        return tileset;

    // Try to do something sensible when the user chooses to make a collection
    if (tileset->isCollection())
        tileset->addTile(QPixmap::fromImage(image), QUrl::fromLocalFile(path));

    if (!newTileset.isEmbedded()) {
        // Save new external tileset
        QScopedPointer<TilesetDocument> tilesetDocument(new TilesetDocument(tileset));
        if (!DocumentManager::instance()->saveDocumentAs(tilesetDocument.data()))
            return SharedTileset();
    }

    return tileset;
}

void MapEditor::setupQuickStamps()
{
    QList<Qt::Key> keys = TileStampManager::quickStampKeys();

    QSignalMapper *selectMapper = new QSignalMapper(this);
    QSignalMapper *createMapper = new QSignalMapper(this);
    QSignalMapper *extendMapper = new QSignalMapper(this);

    for (int i = 0; i < keys.length(); i++) {
        Qt::Key key = keys.at(i);

        // Set up shortcut for selecting this quick stamp
        QShortcut *selectStamp = new QShortcut(key, mMainWindow);
        connect(selectStamp, SIGNAL(activated()), selectMapper, SLOT(map()));
        selectMapper->setMapping(selectStamp, i);

        // Set up shortcut for creating this quick stamp
        QShortcut *createStamp = new QShortcut(Qt::CTRL + key, mMainWindow);
        connect(createStamp, SIGNAL(activated()), createMapper, SLOT(map()));
        createMapper->setMapping(createStamp, i);

        // Set up shortcut for extending this quick stamp
        QShortcut *extendStamp = new QShortcut(Qt::CTRL + Qt::SHIFT + key, mMainWindow);
        connect(extendStamp, SIGNAL(activated()), extendMapper, SLOT(map()));
        extendMapper->setMapping(extendStamp, i);
    }

    connect(selectMapper, SIGNAL(mapped(int)),
            mTileStampManager, SLOT(selectQuickStamp(int)));
    connect(createMapper, SIGNAL(mapped(int)),
            mTileStampManager, SLOT(createQuickStamp(int)));
    connect(extendMapper, SIGNAL(mapped(int)),
            mTileStampManager, SLOT(extendQuickStamp(int)));

    connect(mTileStampManager, SIGNAL(setStamp(TileStamp)),
            this, SLOT(setStamp(TileStamp)));
}

void MapEditor::retranslateUi()
{
    mToolsToolBar->setWindowTitle(tr("Tools"));
    mToolSpecificToolBar->setWindowTitle(tr("Tool Options"));
}

} // namespace Internal
} // namespace Tiled
