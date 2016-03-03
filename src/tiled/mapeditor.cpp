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

#include <QShortcut>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QToolBar>

namespace Tiled {
namespace Internal {

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



MapEditor::MapEditor(QWidget *parent)
    : QMainWindow(parent)
    , mLayerDock(new LayerDock(this))
    , mWidgetStack(new QStackedWidget(this))
    , mCurrentMapDocument(nullptr)
    , mToolManager(new ToolManager(this))
    , mSelectedTool(nullptr)
    , mViewWithTool(nullptr)
{
    setWindowFlags(windowFlags() & ~Qt::Window);

    addDockWidget(Qt::RightDockWidgetArea, mLayerDock);

    setCentralWidget(mWidgetStack);

    mToolBar = new QToolBar(this);
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

    addToolBar(mToolBar);

//    connect(mTilesetDock, SIGNAL(stampCaptured(TileStamp)),
//            this, SLOT(setStamp(TileStamp)));
    connect(mStampBrush, &StampBrush::stampCaptured,
            this, &MapEditor::setStamp);

    //    connect(mRandomButton, SIGNAL(toggled(bool)),
    //            mStampBrush, SLOT(setRandom(bool)));
    //    connect(mRandomButton, SIGNAL(toggled(bool)),
    //            mBucketFillTool, SLOT(setRandom(bool)));

    setSelectedTool(mToolManager->selectedTool());
    connect(mToolManager, &ToolManager::selectedToolChanged,
            this, &MapEditor::setSelectedTool);

    new QShortcut(tr("X"), this, SLOT(flipHorizontally()));
    new QShortcut(tr("Y"), this, SLOT(flipVertically()));
    new QShortcut(tr("Z"), this, SLOT(rotateRight()));
    new QShortcut(tr("Shift+Z"), this, SLOT(rotateLeft()));

    retranslateUi();
}

MapEditor::~MapEditor()
{
    delete mStampBrush;
    mStampBrush = nullptr;

    delete mBucketFillTool;
    mBucketFillTool = nullptr;
}

void MapEditor::addMapDocument(MapDocument *mapDocument)
{
    MapView *view = new MapView;
    MapScene *scene = new MapScene(view); // scene is owned by the view
    MapViewContainer *container = new MapViewContainer(view, mapDocument, mWidgetStack);

    scene->setMapDocument(mapDocument);
    view->setScene(scene);

    mWidgetStack->addWidget(container);
    mWidgetForMap.insert(mapDocument, container);
}

void MapEditor::removeMapDocument(MapDocument *mapDocument)
{
    Q_ASSERT(mWidgetForMap.contains(mapDocument));
    delete mWidgetForMap.take(mapDocument);
}

void MapEditor::setCurrentMapDocument(MapDocument *mapDocument)
{
    if (mCurrentMapDocument == mapDocument)
        return;

    mCurrentMapDocument = mapDocument;

    MapViewContainer *container = mWidgetForMap.value(mapDocument);
    mWidgetStack->setCurrentWidget(container);

    mLayerDock->setMapDocument(mapDocument);
    mToolManager->setMapDocument(mapDocument);

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

MapDocument *MapEditor::currentMapDocument() const
{
    return mCurrentMapDocument;
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

void MapEditor::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange:
        mToolManager->retranslateTools();
        retranslateUi();
        break;
    default:
        break;
    }
}

void MapEditor::cursorChanged(const QCursor &cursor)
{
    if (mViewWithTool)
        mViewWithTool->viewport()->setCursor(cursor);
}

void MapEditor::retranslateUi()
{
    mToolBar->setWindowTitle(tr("Tools"));
}

} // namespace Internal
} // namespace Tiled

#include "mapeditor.moc"
