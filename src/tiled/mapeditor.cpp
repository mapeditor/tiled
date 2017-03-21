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

#include "brokenlinks.h"
#include "bucketfilltool.h"
#include "createellipseobjecttool.h"
#include "createobjecttool.h"
#include "createpolygonobjecttool.h"
#include "createpolylineobjecttool.h"
#include "createrectangleobjecttool.h"
#include "createtextobjecttool.h"
#include "createtileobjecttool.h"
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
#include "objectsdock.h"
#include "objectselectiontool.h"
#include "painttilelayer.h"
#include "preferences.h"
#include "propertiesdock.h"
#include "reversingproxymodel.h"
#include "selectsametiletool.h"
#include "stampbrush.h"
#include "terrain.h"
#include "terrainbrush.h"
#include "terraindock.h"
#include "tile.h"
#include "tileselectiontool.h"
#include "tilesetdock.h"
#include "tilesetmanager.h"
#include "tilestamp.h"
#include "tilestampmanager.h"
#include "tilestampsdock.h"
#include "toolmanager.h"
#include "treeviewcombobox.h"
#include "zoomable.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QIdentityProxyModel>
#include <QLabel>
#include <QMainWindow>
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
 * A proxy model that makes sure no items are checked or checkable.
 *
 * Used in the layer combo box, since the checkboxes can't be used in that
 * context but are otherwise anyway rendered there on Windows.
 */
class UncheckableItemsModel : public QIdentityProxyModel
{
public:
    UncheckableItemsModel(QObject *parent = nullptr)
        : QIdentityProxyModel(parent)
    {
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        if (role == Qt::CheckStateRole)
            return QVariant();

        return QIdentityProxyModel::data(index, role);
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        Qt::ItemFlags rc = QIdentityProxyModel::flags(index);
        return rc & ~Qt::ItemIsUserCheckable;
    }
};


MapEditor::MapEditor(QObject *parent)
    : Editor(parent)
    , mMainWindow(new QMainWindow)
    , mLayerDock(new LayerDock(mMainWindow))
    , mWidgetStack(new QStackedWidget(mMainWindow))
    , mCurrentMapDocument(nullptr)
    , mMapsDock(new MapsDock(mMainWindow))
    , mObjectsDock(new ObjectsDock(mMainWindow))
    , mTilesetDock(new TilesetDock(mMainWindow))
    , mTerrainDock(new TerrainDock(mMainWindow))
    , mMiniMapDock(new MiniMapDock(mMainWindow))
    , mLayerComboBox(new TreeViewComboBox)
    , mUncheckableProxyModel(new UncheckableItemsModel(this))
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
#if QT_VERSION >= 0x050600
    mMainWindow->setDockOptions(mMainWindow->dockOptions() | QMainWindow::GroupedDragging);
#endif
    mMainWindow->setDockNestingEnabled(true);
    mMainWindow->setCentralWidget(mWidgetStack);

    QIcon diceIcon(QLatin1String(":images/24x24/dice.png"));
    diceIcon.addFile(QLatin1String(":images/32x32/dice.png"));

    mRandomButton = new QToolButton(mMainToolBar);
    mRandomButton->setIcon(diceIcon);
    mRandomButton->setCheckable(true);
    mMainToolBar->addWidget(mRandomButton);

    mToolsToolBar = new QToolBar(mMainWindow);
    mToolsToolBar->setObjectName(QLatin1String("toolsToolBar"));

    mStampBrush = new StampBrush(this);
    mTerrainBrush = new TerrainBrush(this);
    mBucketFillTool = new BucketFillTool(this);
    CreateObjectTool *tileObjectsTool = new CreateTileObjectTool(this);
    CreateObjectTool *rectangleObjectsTool = new CreateRectangleObjectTool(this);
    CreateObjectTool *ellipseObjectsTool = new CreateEllipseObjectTool(this);
    CreateObjectTool *polygonObjectsTool = new CreatePolygonObjectTool(this);
    CreateObjectTool *polylineObjectsTool = new CreatePolylineObjectTool(this);
    CreateObjectTool *textObjectsTool = new CreateTextObjectTool(this);

    mToolsToolBar->addAction(mToolManager->registerTool(mStampBrush));
    mToolsToolBar->addAction(mToolManager->registerTool(mTerrainBrush));
    mToolsToolBar->addAction(mToolManager->registerTool(mBucketFillTool));
    mToolsToolBar->addAction(mToolManager->registerTool(new Eraser(this)));
    mToolsToolBar->addAction(mToolManager->registerTool(new TileSelectionTool(this)));
    mToolsToolBar->addAction(mToolManager->registerTool(new MagicWandTool(this)));
    mToolsToolBar->addAction(mToolManager->registerTool(new SelectSameTileTool(this)));
    mToolsToolBar->addSeparator();
    mToolsToolBar->addAction(mToolManager->registerTool(new ObjectSelectionTool(this)));
    mToolsToolBar->addAction(mToolManager->registerTool(new EditPolygonTool(this)));
    mToolsToolBar->addAction(mToolManager->registerTool(rectangleObjectsTool));
    mToolsToolBar->addAction(mToolManager->registerTool(ellipseObjectsTool));
    mToolsToolBar->addAction(mToolManager->registerTool(polygonObjectsTool));
    mToolsToolBar->addAction(mToolManager->registerTool(polylineObjectsTool));
    mToolsToolBar->addAction(mToolManager->registerTool(tileObjectsTool));
    mToolsToolBar->addAction(mToolManager->registerTool(textObjectsTool));
    mToolsToolBar->addSeparator();
    mToolsToolBar->addAction(mToolManager->registerTool(new LayerOffsetTool(this)));

    mMainWindow->addToolBar(mMainToolBar);
    mMainWindow->addToolBar(mToolsToolBar);

    mPropertiesDock = new PropertiesDock(mMainWindow);
    mTileStampsDock = new TileStampsDock(mTileStampManager, mMainWindow);

    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mLayerDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mPropertiesDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mMapsDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mObjectsDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mMiniMapDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mTerrainDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mTilesetDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mTileStampsDock);

    mMainWindow->tabifyDockWidget(mMiniMapDock, mObjectsDock);
    mMainWindow->tabifyDockWidget(mObjectsDock, mLayerDock);
    mMainWindow->tabifyDockWidget(mTerrainDock, mTilesetDock);

    mMapsDock->setVisible(false);
    mTileStampsDock->setVisible(false);

    mUncheckableProxyModel->setSourceModel(mReversingProxyModel);
    mLayerComboBox->setModel(mUncheckableProxyModel);
    mLayerComboBox->setMinimumContentsLength(10);
    mLayerComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(mLayerComboBox,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this,
            &MapEditor::layerComboActivated);

    mMainWindow->statusBar()->addPermanentWidget(mLayerComboBox);
    mMainWindow->statusBar()->addPermanentWidget(mZoomComboBox);
    mMainWindow->statusBar()->addWidget(mStatusInfoLabel);

    connect(mWidgetStack, &QStackedWidget::currentChanged, this, &MapEditor::currentWidgetChanged);
    connect(mToolManager, &ToolManager::statusInfoChanged, this, &MapEditor::updateStatusInfoLabel);
    connect(mTilesetDock,
            &TilesetDock::currentTileChanged,
            tileObjectsTool,
            &CreateObjectTool::setTile);
    connect(mTilesetDock, &TilesetDock::stampCaptured, this, &MapEditor::setStamp);
    connect(mStampBrush, &StampBrush::stampCaptured, this, &MapEditor::setStamp);

    connect(mRandomButton, &QToolButton::toggled, mStampBrush, &StampBrush::setRandom);
    connect(mRandomButton, &QToolButton::toggled, mBucketFillTool, &BucketFillTool::setRandom);

    connect(mTerrainDock,
            &TerrainDock::currentTerrainChanged,
            mTerrainBrush,
            &TerrainBrush::setTerrain);
    connect(mTerrainDock, &TerrainDock::selectTerrainBrush, this, &MapEditor::selectTerrainBrush);
    connect(mTerrainBrush,
            &TerrainBrush::terrainCaptured,
            mTerrainDock,
            &TerrainDock::setCurrentTerrain);

    connect(mTileStampsDock, SIGNAL(setStamp(TileStamp)), this, SLOT(setStamp(TileStamp)));

    setSelectedTool(mToolManager->selectedTool());
    connect(mToolManager, &ToolManager::selectedToolChanged, this, &MapEditor::setSelectedTool);

    QShortcut *flipHorizontallyShortcut = new QShortcut(tr("X"), mMainWindow);
    QShortcut *flipVerticallyShortcut = new QShortcut(tr("Y"), mMainWindow);
    QShortcut *rotateRightShortcut = new QShortcut(tr("Z"), mMainWindow);
    QShortcut *rotateLeftShortcut = new QShortcut(tr("Shift+Z"), mMainWindow);

    connect(flipHorizontallyShortcut, &QShortcut::activated, this, &MapEditor::flipHorizontally);
    connect(flipVerticallyShortcut, &QShortcut::activated, this, &MapEditor::flipVertically);
    connect(rotateRightShortcut, &QShortcut::activated, this, &MapEditor::rotateRight);
    connect(rotateLeftShortcut, &QShortcut::activated, this, &MapEditor::rotateLeft);

    setupQuickStamps();
    retranslateUi();
    connect(
        Preferences::instance(), &Preferences::languageChanged, this, &MapEditor::retranslateUi);

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
    MapDocument *mapDocument = qobject_cast<MapDocument *>(document);
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

        const int hor = mapState.value(QLatin1String("scrollX")).toInt();
        const int ver = mapState.value(QLatin1String("scrollY")).toInt();
        view->horizontalScrollBar()->setSliderPosition(hor);
        view->verticalScrollBar()->setSliderPosition(ver);

        int layerIndex = mapState.value(QLatin1String("selectedLayer")).toInt();
        if (Layer *layer = layerAtGlobalIndex(mapDocument->map(), layerIndex))
            mapDocument->setCurrentLayer(layer);
    }
}

void MapEditor::removeDocument(Document *document)
{
    MapDocument *mapDocument = qobject_cast<MapDocument *>(document);
    Q_ASSERT(mapDocument);
    Q_ASSERT(mWidgetForMap.contains(mapDocument));

    MapView *mapView = mWidgetForMap.take(mapDocument);

    // remember the state of this map before deleting the view
    if (!mapDocument->fileName().isEmpty()) {
        QVariantMap mapState;
        mapState.insert(QLatin1String("scale"), mapView->zoomable()->scale());
        mapState.insert(QLatin1String("scrollX"), mapView->horizontalScrollBar()->sliderPosition());
        mapState.insert(QLatin1String("scrollY"), mapView->verticalScrollBar()->sliderPosition());
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
    MapDocument *mapDocument = qobject_cast<MapDocument *>(document);
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
    mObjectsDock->setMapDocument(mapDocument);
    mTilesetDock->setMapDocument(mapDocument);
    mTerrainDock->setDocument(mapDocument);
    mMiniMapDock->setMapDocument(mapDocument);

    if (mapDocument) {
        connect(mapDocument,
                &MapDocument::currentLayerChanged,
                this,
                &MapEditor::updateLayerComboIndex);
        //        connect(mapDocument, SIGNAL(selectedAreaChanged(QRegion,QRegion)),
        //                SLOT(updateActions()));
        //        connect(mapDocument, SIGNAL(selectedObjectsChanged()),
        //                SLOT(updateActions()));

        if (mapView) {
            mZoomable = mapView->zoomable();
            mZoomable->setComboBox(mZoomComboBox);
        }

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
    return QList<QToolBar *>{mMainToolBar, mToolsToolBar};
}

QList<QDockWidget *> MapEditor::dockWidgets() const
{
    return QList<QDockWidget *>{mPropertiesDock,
                                mLayerDock,
                                mMapsDock,
                                mObjectsDock,
                                mTilesetDock,
                                mTerrainDock,
                                mMiniMapDock,
                                mTileStampsDock};
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
        disconnect(mSelectedTool, &AbstractTool::cursorChanged, this, &MapEditor::cursorChanged);
    }

    mSelectedTool = tool;

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
        connect(tool, &AbstractTool::cursorChanged, this, &MapEditor::cursorChanged);
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
            TileLayer *source = static_cast<TileLayer *>(layer);
            TileLayer *target = currentLayer->asTileLayer();

            if (target) {
                // Paste onto the current layer, using the same position as where
                // the copied piece came from.
                auto undoStack = mCurrentMapDocument->undoStack();
                undoStack->push(new PaintTileLayer(
                    mCurrentMapDocument, target, source->x(), source->y(), source));
            }
        } else {
            // Reset selection and paste into the stamp brush
            MapDocumentActionHandler::instance()->selectNone();
            layer->setPosition(0, 0); // Make sure the tile layer is at origin
            Map *stamp = map.take();  // TileStamp will take ownership
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

    // When selecting a new stamp, it makes sense to switch to a stamp tool
    AbstractTool *selectedTool = mToolManager->selectedTool();
    if (selectedTool != mStampBrush && selectedTool != mBucketFillTool)
        mToolManager->selectTool(mStampBrush);

    mTilesetDock->selectTilesInStamp(stamp);
}

void MapEditor::selectTerrainBrush()
{
    mToolManager->selectTool(mTerrainBrush);
}

void MapEditor::currentWidgetChanged()
{
    auto mapView = static_cast<MapView *>(mWidgetStack->currentWidget());
    setCurrentDocument(mapView ? mapView->mapScene()->mapDocument() : nullptr);
}

// void MapEditor::changeEvent(QEvent *event)
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
    const QModelIndex reversedIndex = mUncheckableProxyModel->mapToSource(comboIndex);
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
        index = mUncheckableProxyModel->mapFromSource(reversedIndex);
    }

    mLayerComboBox->setCurrentModelIndex(index);
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

    connect(selectMapper, SIGNAL(mapped(int)), mTileStampManager, SLOT(selectQuickStamp(int)));
    connect(createMapper, SIGNAL(mapped(int)), mTileStampManager, SLOT(createQuickStamp(int)));
    connect(extendMapper, SIGNAL(mapped(int)), mTileStampManager, SLOT(extendQuickStamp(int)));

    connect(mTileStampManager, SIGNAL(setStamp(TileStamp)), this, SLOT(setStamp(TileStamp)));
}

void MapEditor::retranslateUi()
{
    mRandomButton->setToolTip(tr("Random Mode"));
    mRandomButton->setShortcut(QKeySequence(tr("D")));

    mToolsToolBar->setWindowTitle(tr("Tools"));
}

} // namespace Internal
} // namespace Tiled
