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
#include "createobjecttool.h"
#include "createrectangleobjecttool.h"
#include "createellipseobjecttool.h"
#include "createtileobjecttool.h"
#include "createpolygonobjecttool.h"
#include "createpolylineobjecttool.h"
#include "layerdock.h"
#include "mapscene.h"
#include "mapview.h"
#include "tilestamp.h"
#include "layeroffsettool.h"
#include "eraser.h"
#include "tile.h"
#include "tileselectiontool.h"
#include "magicwandtool.h"
#include "selectsametiletool.h"
#include "filechangedwarning.h"
#include "toolmanager.h"
#include "objectselectiontool.h"
#include "editpolygontool.h"
#include "stampbrush.h"
#include "terrainbrush.h"
#include "bucketfilltool.h"
#include "mapsdock.h"
#include "propertiesdock.h"
#include "tilesetdock.h"
#include "tilestampsdock.h"
#include "terraindock.h"
#include "objectsdock.h"
#include "minimapdock.h"
#include "tilestampmanager.h"
#include "zoomable.h"
#include "layermodel.h"

#include <QShortcut>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QToolBar>
#include <QIdentityProxyModel>
#include <QComboBox>
#include <QStatusBar>
#include <QSignalMapper>
#include <QMainWindow>

namespace Tiled {
namespace Internal {

namespace {

/**
 * A model that is always empty.
 */
class EmptyModel : public QAbstractListModel
{
public:
    EmptyModel(QObject *parent = nullptr)
        : QAbstractListModel(parent)
    {}

    int rowCount(const QModelIndex &) const override
    { return 0; }

    QVariant data(const QModelIndex &, int) const override
    { return QVariant(); }
};

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
    {}

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

static EmptyModel emptyModel;
static UncheckableItemsModel uncheckableLayerModel;

} // anonymous namespace


class MapViewContainer : public QWidget
{
    Q_OBJECT

public:
    MapViewContainer(MapView *mapView,
                     MapDocument *mapDocument,
                     QWidget *parent = nullptr)
        : QWidget(parent)
        , mMapView(mapView)
        , mWarning(new FileChangedWarning)
        , mBrokenLinksModel(new BrokenLinksModel(mapDocument, this))
        , mBrokenLinksWidget(nullptr)
    {
        mWarning->setVisible(false);

        QVBoxLayout *layout = new QVBoxLayout(this);
        layout->setMargin(0);
        layout->setSpacing(0);

        if (mBrokenLinksModel->hasBrokenLinks()) {
            mBrokenLinksWidget = new BrokenLinksWidget(mBrokenLinksModel, this);
            layout->addWidget(mBrokenLinksWidget);

            connect(mBrokenLinksWidget, &BrokenLinksWidget::ignore,
                    this, &MapViewContainer::deleteBrokenLinksWidget);
        }

        connect(mBrokenLinksModel, &BrokenLinksModel::hasBrokenLinksChanged,
                this, &MapViewContainer::hasBrokenLinksChanged);

        layout->addWidget(mapView);
        layout->addWidget(mWarning);

        connect(mWarning, &FileChangedWarning::reload, this, &MapViewContainer::reload);
        connect(mWarning, &FileChangedWarning::ignore, mWarning, &FileChangedWarning::hide);
    }

    MapView *mapView() const { return mMapView; }

    void setFileChangedWarningVisible(bool visible)
    { mWarning->setVisible(visible); }

signals:
    void reload();

private slots:
    void hasBrokenLinksChanged(bool hasBrokenLinks)
    {
        if (!hasBrokenLinks)
            deleteBrokenLinksWidget();
    }

    void deleteBrokenLinksWidget()
    {
        if (mBrokenLinksWidget) {
            mBrokenLinksWidget->deleteLater();
            mBrokenLinksWidget = nullptr;
        }
    }

private:
    MapView *mMapView;

    FileChangedWarning *mWarning;
    BrokenLinksModel *mBrokenLinksModel;
    BrokenLinksWidget *mBrokenLinksWidget;
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
    , mLayerComboBox(new QComboBox)
    , mZoomable(nullptr)
    , mZoomComboBox(new QComboBox)
    , mStatusInfoLabel(new QLabel)
    , mToolManager(new ToolManager(this))
    , mSelectedTool(nullptr)
    , mViewWithTool(nullptr)
    , mTileStampManager(new TileStampManager(mToolManager, this))
{
    mMainWindow->setWindowFlags(mMainWindow->windowFlags() & ~Qt::Window);
    mMainWindow->setCentralWidget(mWidgetStack);

    mToolBar = new QToolBar(mMainWindow);
    mToolBar->setObjectName(QLatin1String("toolsToolBar"));

    mStampBrush = new StampBrush(this);
    mTerrainBrush = new TerrainBrush(this);
    mBucketFillTool = new BucketFillTool(this);
    CreateObjectTool *tileObjectsTool = new CreateTileObjectTool(this);
    CreateObjectTool *rectangleObjectsTool = new CreateRectangleObjectTool(this);
    CreateObjectTool *ellipseObjectsTool = new CreateEllipseObjectTool(this);
    CreateObjectTool *polygonObjectsTool = new CreatePolygonObjectTool(this);
    CreateObjectTool *polylineObjectsTool = new CreatePolylineObjectTool(this);

    mToolBar->addAction(mToolManager->registerTool(mStampBrush));
    mToolBar->addAction(mToolManager->registerTool(mTerrainBrush));
    mToolBar->addAction(mToolManager->registerTool(mBucketFillTool));
    mToolBar->addAction(mToolManager->registerTool(new Eraser(this)));
    mToolBar->addAction(mToolManager->registerTool(new TileSelectionTool(this)));
    mToolBar->addAction(mToolManager->registerTool(new MagicWandTool(this)));
    mToolBar->addAction(mToolManager->registerTool(new SelectSameTileTool(this)));
    mToolBar->addSeparator();
    mToolBar->addAction(mToolManager->registerTool(new ObjectSelectionTool(this)));
    mToolBar->addAction(mToolManager->registerTool(new EditPolygonTool(this)));
    mToolBar->addAction(mToolManager->registerTool(rectangleObjectsTool));
    mToolBar->addAction(mToolManager->registerTool(ellipseObjectsTool));
    mToolBar->addAction(mToolManager->registerTool(polygonObjectsTool));
    mToolBar->addAction(mToolManager->registerTool(polylineObjectsTool));
    mToolBar->addAction(mToolManager->registerTool(tileObjectsTool));
    mToolBar->addSeparator();
    mToolBar->addAction(mToolManager->registerTool(new LayerOffsetTool(this)));

    mMainWindow->addToolBar(mToolBar);

    mPropertiesDock = new PropertiesDock(mMainWindow);
    TileStampsDock *tileStampsDock = new TileStampsDock(mTileStampManager, mMainWindow);

    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mLayerDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mPropertiesDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, mMapsDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mObjectsDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mMiniMapDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mTerrainDock);
    mMainWindow->addDockWidget(Qt::RightDockWidgetArea, mTilesetDock);
    mMainWindow->addDockWidget(Qt::LeftDockWidgetArea, tileStampsDock);

    mMainWindow->tabifyDockWidget(mMiniMapDock, mObjectsDock);
    mMainWindow->tabifyDockWidget(mTerrainDock, mTilesetDock);

    mMapsDock->setVisible(false);
    tileStampsDock->setVisible(false);

    mLayerComboBox->setMinimumContentsLength(10);
    mLayerComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    connect(mLayerComboBox, SIGNAL(activated(int)),
            this, SLOT(layerComboActivated(int)));

    mMainWindow->statusBar()->addPermanentWidget(mLayerComboBox);
    mMainWindow->statusBar()->addPermanentWidget(mZoomComboBox);
    mMainWindow->statusBar()->addWidget(mStatusInfoLabel);

    connect(mWidgetStack, &QStackedWidget::currentChanged, this, &MapEditor::currentWidgetChanged);
    connect(mToolManager, &ToolManager::statusInfoChanged, this, &MapEditor::updateStatusInfoLabel);
    connect(mTilesetDock, &TilesetDock::currentTileChanged, tileObjectsTool, &CreateObjectTool::setTile);
    connect(mTilesetDock, &TilesetDock::stampCaptured, this, &MapEditor::setStamp);
    connect(mStampBrush, &StampBrush::stampCaptured, this, &MapEditor::setStamp);

    //    connect(mRandomButton, SIGNAL(toggled(bool)), mStampBrush, SLOT(setRandom(bool)));
    //    connect(mRandomButton, SIGNAL(toggled(bool)), mBucketFillTool, SLOT(setRandom(bool)));

    setSelectedTool(mToolManager->selectedTool());
    connect(mToolManager, &ToolManager::selectedToolChanged,
            this, &MapEditor::setSelectedTool);

    // todo: connect these signals differently
    new QShortcut(tr("X"), mMainWindow, SLOT(flipHorizontally()));
    new QShortcut(tr("Y"), mMainWindow, SLOT(flipVertically()));
    new QShortcut(tr("Z"), mMainWindow, SLOT(rotateRight()));
    new QShortcut(tr("Shift+Z"), mMainWindow, SLOT(rotateLeft()));

    setupQuickStamps();
    retranslateUi();
}

MapEditor::~MapEditor()
{
}

void MapEditor::addDocument(Document *document)
{
    MapDocument *mapDocument = qobject_cast<MapDocument*>(document);
    Q_ASSERT(mapDocument);

    MapView *view = new MapView;
    MapScene *scene = new MapScene(view); // scene is owned by the view
    MapViewContainer *container = new MapViewContainer(view, mapDocument, mWidgetStack);

    scene->setMapDocument(mapDocument);
    view->setScene(scene);

    mWidgetStack->addWidget(container);
    mWidgetForMap.insert(mapDocument, container);
}

void MapEditor::removeDocument(Document *document)
{
    MapDocument *mapDocument = qobject_cast<MapDocument*>(document);
    Q_ASSERT(mapDocument);
    Q_ASSERT(mWidgetForMap.contains(mapDocument));

    MapViewContainer *mapViewContainer = mWidgetForMap.take(mapDocument);
    // remove first, to keep it valid while the current widget changes
    mWidgetStack->removeWidget(mapViewContainer);
    delete mapViewContainer;
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

    MapViewContainer *container = mWidgetForMap.value(mapDocument);
    if (container)
        mWidgetStack->setCurrentWidget(container);

    mLayerDock->setMapDocument(mapDocument);
    mToolManager->setMapDocument(mapDocument);

    if (mZoomable) {
        mZoomable->connectToComboBox(nullptr);

        disconnect(mZoomable, SIGNAL(scaleChanged(qreal)),
                   this, SLOT(updateZoomLabel()));
    }
    mZoomable = nullptr;

    mPropertiesDock->setDocument(mapDocument);
    mObjectsDock->setMapDocument(mapDocument);
    mTilesetDock->setMapDocument(mapDocument);
    mTerrainDock->setMapDocument(mapDocument);
    mMiniMapDock->setMapDocument(mapDocument);

    if (mapDocument) {
        connect(mapDocument, SIGNAL(currentLayerIndexChanged(int)),
                SLOT(updateLayerComboIndex()));
//        connect(mapDocument, SIGNAL(selectedAreaChanged(QRegion,QRegion)),
//                SLOT(updateActions()));
//        connect(mapDocument, SIGNAL(selectedObjectsChanged()),
//                SLOT(updateActions()));

        if (MapView *mapView = currentMapView()) {
            mZoomable = mapView->zoomable();
            mZoomable->connectToComboBox(mZoomComboBox);

            connect(mZoomable, SIGNAL(scaleChanged(qreal)),
                    this, SLOT(updateZoomLabel()));
        }

        uncheckableLayerModel.setSourceModel(mapDocument->layerModel());
        mLayerComboBox->setModel(&uncheckableLayerModel);
    } else {
        mLayerComboBox->setModel(&emptyModel);
    }

    mLayerComboBox->setEnabled(mapDocument);

    // Take the currently active tool to the new map view
    if (mViewWithTool) {
        MapScene *mapScene = mViewWithTool->mapScene();
        mapScene->disableSelectedTool();
        mViewWithTool = nullptr;
    }
    if (MapView *mapView = currentMapView()) {
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

MapView *MapEditor::viewForDocument(MapDocument *mapDocument) const
{
    MapViewContainer *container = mWidgetForMap.value(mapDocument);
    return container ? container->mapView() : nullptr;
}

MapView *MapEditor::currentMapView() const
{
    MapViewContainer *container = mWidgetForMap.value(mCurrentMapDocument);
    return container ? container->mapView() : nullptr;
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
    }
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

//    mTilesetDock->selectTilesInStamp(stamp);
}

/**
 * Sets the terrain brush.
 */
void MapEditor::setTerrainBrush(const Terrain *terrain)
{
    mTerrainBrush->setTerrain(terrain);

    // When selecting a new terrain, it makes sense to switch to a terrain brush tool
    AbstractTool *selectedTool = mToolManager->selectedTool();
    if (selectedTool != mTerrainBrush)
        mToolManager->selectTool(mTerrainBrush);
}

void MapEditor::currentWidgetChanged()
{
    auto container = static_cast<MapViewContainer*>(mWidgetStack->currentWidget());
    setCurrentDocument(container ? container->mapView()->mapScene()->mapDocument() : nullptr);
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

void MapEditor::updateZoomLabel()
{
    MapView *mapView = currentMapView();
    Zoomable *zoomable = mapView ? mapView->zoomable() : nullptr;

    if (zoomable) {
        mZoomComboBox->setEnabled(true);
    } else {
        int index = mZoomComboBox->findData((qreal)1.0);
        mZoomComboBox->setCurrentIndex(index);
        mZoomComboBox->setEnabled(false);
    }
}

void MapEditor::layerComboActivated(int index)
{
    if (index == -1)
        return;
    if (!mCurrentMapDocument)
        return;

    int layerIndex = mCurrentMapDocument->layerModel()->toLayerIndex(index);

    if (layerIndex != mCurrentMapDocument->currentLayerIndex())
        mCurrentMapDocument->setCurrentLayerIndex(layerIndex);
}

void MapEditor::updateLayerComboIndex()
{
    int layerComboIndex = -1;

    if (mCurrentMapDocument) {
        int layerIndex = mCurrentMapDocument->currentLayerIndex();
        if (layerIndex != -1)
            layerComboIndex = mCurrentMapDocument->layerModel()->layerIndexToRow(layerIndex);
    }

    mLayerComboBox->setCurrentIndex(layerComboIndex);
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
    mToolBar->setWindowTitle(tr("Tools"));
}

} // namespace Internal
} // namespace Tiled

#include "mapeditor.moc"
