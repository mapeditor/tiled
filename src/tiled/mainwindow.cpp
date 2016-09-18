/*
 * mainwindow.cpp
 * Copyright 2008-2015, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2009-2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2009, Dennis Honeyman <arcticuno@gmail.com>
 * Copyright 2009, Christian Henz <chrhenz@gmx.de>
 * Copyright 2010, Andrew G. Crowell <overkill9999@gmail.com>
 * Copyright 2010-2011, Stefan Beller <stefanbeller@googlemail.com>
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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "aboutdialog.h"
#include "addremovemapobject.h"
#include "automappingmanager.h"
#include "addremovetileset.h"
#include "clipboardmanager.h"
#include "createobjecttool.h"
#include "createrectangleobjecttool.h"
#include "createellipseobjecttool.h"
#include "createtileobjecttool.h"
#include "createpolygonobjecttool.h"
#include "createpolylineobjecttool.h"
#include "documentmanager.h"
#include "editpolygontool.h"
#include "eraser.h"
#include "erasetiles.h"
#include "exportasimagedialog.h"
#include "bucketfilltool.h"
#include "filltiles.h"
#include "languagemanager.h"
#include "layer.h"
#include "layerdock.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"
#include "mapobject.h"
#include "maprenderer.h"
#include "mapsdock.h"
#include "mapscene.h"
#include "mapview.h"
#include "newmapdialog.h"
#include "newtilesetdialog.h"
#include "pluginmanager.h"
#include "resizedialog.h"
#include "objectselectiontool.h"
#include "objectgroup.h"
#include "offsetmapdialog.h"
#include "patreondialog.h"
#include "preferences.h"
#include "preferencesdialog.h"
#include "propertiesdock.h"
#include "stampbrush.h"
#include "terrainbrush.h"
#include "tilelayer.h"
#include "tileselectiontool.h"
#include "tileset.h"
#include "tilesetdock.h"
#include "tilesetmanager.h"
#include "tilestampmanager.h"
#include "tilestampsdock.h"
#include "terraindock.h"
#include "toolmanager.h"
#include "tmxmapreader.h"
#include "tmxmapwriter.h"
#include "undodock.h"
#include "utils.h"
#include "zoomable.h"
#include "commandbutton.h"
#include "objectsdock.h"
#include "minimapdock.h"
#include "consoledock.h"
#include "tileanimationeditor.h"
#include "tilecollisioneditor.h"
#include "imagemovementtool.h"
#include "magicwandtool.h"
#include "selectsametiletool.h"

#include "rtbmapsettings.h"
#include "rtbcreateobjecttool.h"
#include "rtbtilebutton.h"
#include "rtbtileselectionmanager.h"
#include "rtbvalidatordock.h"
#include "rtbvalidator.h"
#include "rtbselectareatool.h"
#include "rtbinserttool.h"
#include "rtbcore.h"
#include "rtbtutorial.h"
#include "rtbtutorialdock.h"

#ifdef Q_OS_MAC
#include "macsupport.h"
#endif

#include <QMimeData>
#include <QCloseEvent>
#include <QComboBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QSessionManager>
#include <QTextStream>
#include <QUndoGroup>
#include <QUndoStack>
#include <QUndoView>
#include <QImageReader>
#include <QRegExp>
#include <QSignalMapper>
#include <QShortcut>
#include <QToolButton>

using namespace Tiled;
using namespace Tiled::Internal;
using namespace Tiled::Utils;

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , mUi(new Ui::MainWindow)
    , mMapDocument(0)
    , mActionHandler(new MapDocumentActionHandler(this))
    , mLayerDock(new LayerDock(this))
    , mMapsDock(new MapsDock(this))
    , mMiniMapDock(new MiniMapDock(this))
    , mCurrentLayerLabel(new QLabel)
    , mZoomable(0)
    , mZoomComboBox(new QComboBox)
    , mStatusInfoLabel(new QLabel)
    , mDocumentManager(DocumentManager::instance())
    , mToolManager(new ToolManager(this))
    , mTileStampManager(new TileStampManager(*mToolManager, this))
    , mTileSelectionManager(new RTBTileSelectionManager(this))
    , mValidatorDock(new RTBValidatorDock(this))
    , mValidator(new RTBValidator(mValidatorDock))
    , mTutorialDock(new RTBTutorialDock(this))
{
    mUi->setupUi(this);
    setCentralWidget(mDocumentManager->widget());

#ifdef Q_OS_MAC
    MacSupport::addFullscreen(this);
#endif

    Preferences *preferences = Preferences::instance();
    mSettings = preferences->settings();

    QIcon redoIcon(QLatin1String(":images/16x16/edit-redo.png"));
    QIcon undoIcon(QLatin1String(":images/16x16/edit-undo.png"));

#ifndef Q_OS_MAC
    QIcon tiledIcon(QLatin1String(":images/16x16/tiled.png"));
    tiledIcon.addFile(QLatin1String(":images/32x32/tiled.png"));
    setWindowIcon(tiledIcon);
#endif

    // Add larger icon versions for actions used in the tool bar
    QIcon newIcon = mUi->actionNew->icon();
    QIcon openIcon = mUi->actionOpen->icon();
    QIcon saveIcon = mUi->actionSave->icon();
    newIcon.addFile(QLatin1String(":images/24x24/document-new.png"));
    openIcon.addFile(QLatin1String(":images/24x24/document-open.png"));
    saveIcon.addFile(QLatin1String(":images/24x24/document-save.png"));
    redoIcon.addFile(QLatin1String(":images/24x24/edit-redo.png"));
    undoIcon.addFile(QLatin1String(":images/24x24/edit-undo.png"));
    mUi->actionNew->setIcon(newIcon);
    mUi->actionOpen->setIcon(openIcon);
    mUi->actionSave->setIcon(saveIcon);

    QUndoGroup *undoGroup = mDocumentManager->undoGroup();
    QAction *undoAction = undoGroup->createUndoAction(this, tr("Undo"));
    QAction *redoAction = undoGroup->createRedoAction(this, tr("Redo"));
    mUi->mainToolBar->setToolButtonStyle(Qt::ToolButtonFollowStyle);
    mUi->actionNew->setPriority(QAction::LowPriority);
    redoAction->setPriority(QAction::LowPriority);
    redoAction->setIcon(redoIcon);
    undoAction->setIcon(undoIcon);
    redoAction->setIconText(tr("Redo"));
    undoAction->setIconText(tr("Undo"));
    connect(undoGroup, SIGNAL(cleanChanged(bool)), SLOT(updateWindowTitle()));

    UndoDock *undoDock = new UndoDock(undoGroup, this);
    mPropertiesDock = new PropertiesDock(this);

    addDockWidget(Qt::RightDockWidgetArea, mLayerDock);
    addDockWidget(Qt::LeftDockWidgetArea, mPropertiesDock);
    addDockWidget(Qt::LeftDockWidgetArea, undoDock);
    addDockWidget(Qt::LeftDockWidgetArea, mMapsDock);
    addDockWidget(Qt::RightDockWidgetArea, mMiniMapDock);

    addDockWidget(Qt::LeftDockWidgetArea, mValidatorDock);
    addDockWidget(Qt::RightDockWidgetArea, mTutorialDock);

    tabifyDockWidget(mMiniMapDock, mLayerDock);
    tabifyDockWidget(undoDock, mMapsDock);

    // These dock widgets may not be immediately useful to many people, so
    // they are hidden by default.
    undoDock->setVisible(false);
    mMapsDock->setVisible(false);

    statusBar()->addPermanentWidget(mZoomComboBox);

    mUi->actionNew->setShortcuts(QKeySequence::New);
    mUi->actionOpen->setShortcuts(QKeySequence::Open);
    mUi->actionSave->setShortcuts(QKeySequence::Save);
    mUi->actionSaveAs->setShortcuts(QKeySequence::SaveAs);
    mUi->actionClose->setShortcuts(QKeySequence::Close);
    mUi->actionQuit->setShortcuts(QKeySequence::Quit);
    mUi->actionCut->setShortcuts(QKeySequence::Cut);
    mUi->actionCopy->setShortcuts(QKeySequence::Copy);
    mUi->actionPaste->setShortcuts(QKeySequence::Paste);
    mUi->actionDelete->setShortcuts(QKeySequence::Delete);
    undoAction->setShortcuts(QKeySequence::Undo);
    redoAction->setShortcuts(QKeySequence::Redo);

    mUi->actionShowGrid->setChecked(preferences->showGrid());
    mUi->actionShowTileObjectOutlines->setChecked(preferences->showTileObjectOutlines());
    mUi->actionShowTileAnimations->setChecked(preferences->showTileAnimations());
    mUi->actionSnapToGrid->setChecked(preferences->snapToGrid());
    mUi->actionHighlightCurrentLayer->setChecked(preferences->highlightCurrentLayer());

    QShortcut *reloadTilesetsShortcut = new QShortcut(QKeySequence(tr("Ctrl+T")), this);
    connect(reloadTilesetsShortcut, SIGNAL(activated()),
            this, SLOT(reloadTilesets()));

    // Make sure Ctrl+= also works for zooming in
    QList<QKeySequence> keys = QKeySequence::keyBindings(QKeySequence::ZoomIn);
    keys += QKeySequence(tr("Ctrl+="));
    keys += QKeySequence(tr("+"));
    mUi->actionZoomIn->setShortcuts(keys);
    keys = QKeySequence::keyBindings(QKeySequence::ZoomOut);
    keys += QKeySequence(tr("-"));
    mUi->actionZoomOut->setShortcuts(keys);

    mUi->menuEdit->insertAction(mUi->actionCut, undoAction);
    mUi->menuEdit->insertAction(mUi->actionCut, redoAction);
    mUi->menuEdit->insertSeparator(mUi->actionCut);
    mUi->menuEdit->insertAction(mUi->actionPreferences,
                                mActionHandler->actionSelectAll());
    mUi->menuEdit->insertAction(mUi->actionPreferences,
                                mActionHandler->actionSelectNone());
    mUi->menuEdit->insertSeparator(mUi->actionPreferences);
    mUi->mainToolBar->addAction(undoAction);
    mUi->mainToolBar->addAction(redoAction);

    mUi->mainToolBar->addSeparator();

    mShowMapProperties = new QAction(QIcon(QLatin1String(":/images/24x24/document-properties.png"))
                                                   , QLatin1String("Show Map Properties  (P)"), this);
    mShowMapProperties->setShortcut(QKeySequence(Qt::Key_P));
    mUi->mainToolBar->addAction(mShowMapProperties);

    connect(mShowMapProperties, SIGNAL(triggered()),
            SLOT(editMapProperties()));

    QToolButton *showHidePropButton = new QToolButton(this);
    showHidePropButton->setToolTip(tr("Show/Hide Property Visualization (V)"));
    showHidePropButton->setIcon(QIcon(QLatin1String("://rtb_resources/icons/action_visualhelper.png")));
    showHidePropButton->setCheckable(true);
    showHidePropButton->setShortcut(QKeySequence(Qt::Key_V));
    mUi->mainToolBar->addWidget(showHidePropButton);

    bool showProp = preferences->showPropertyVisualization();
    setShowPropVisualization(showProp);
    showHidePropButton->setChecked(showProp);
    connect(showHidePropButton, SIGNAL(toggled(bool)),
            this, SLOT(setShowPropVisualization(bool)));

    mPlayLevelAction = new QAction(QIcon(QLatin1String("://rtb_resources/icons/action_playlevel.png"))
                               , QLatin1String("Play Map (F5)"), this);
    mPlayLevelAction->setShortcut(QKeySequence(Qt::Key_F5));
    mPlayLevelAction->setEnabled(false);
    mUi->mainToolBar->addAction(mPlayLevelAction);
    connect(mPlayLevelAction, SIGNAL(triggered()), SLOT(buildMap()));

    mLayerMenu = new QMenu(tr("&Layer"), this);
    mLayerMenu->addAction(mActionHandler->actionLayerProperties());

    menuBar()->insertMenu(mUi->menuHelp->menuAction(), mLayerMenu);

    connect(mUi->actionNew, SIGNAL(triggered()), SLOT(newMap()));
    connect(mUi->actionOpen, SIGNAL(triggered()), SLOT(openFile()));
    connect(mUi->actionClearRecentFiles, SIGNAL(triggered()),
            SLOT(clearRecentFiles()));
    connect(mUi->actionSave, SIGNAL(triggered()), SLOT(saveFile()));
    connect(mUi->actionSaveAs, SIGNAL(triggered()), SLOT(saveFileAsJSON()));
    connect(mUi->actionSaveAll, SIGNAL(triggered()), SLOT(saveAll()));
    connect(mUi->actionExportAsImage, SIGNAL(triggered()), SLOT(exportAsImage()));
    connect(mUi->actionReload, SIGNAL(triggered()), SLOT(reload()));
    connect(mUi->actionClose, SIGNAL(triggered()), SLOT(closeFile()));
    connect(mUi->actionCloseAll, SIGNAL(triggered()), SLOT(closeAllFiles()));
    connect(mUi->actionQuit, SIGNAL(triggered()), SLOT(close()));

    connect(mUi->actionCut, SIGNAL(triggered()), SLOT(cut()));
    connect(mUi->actionCopy, SIGNAL(triggered()), SLOT(copy()));
    connect(mUi->actionPaste, SIGNAL(triggered()), SLOT(paste()));
    connect(mUi->actionDelete, SIGNAL(triggered()), SLOT(delete_()));
    connect(mUi->actionPreferences, SIGNAL(triggered()),
            SLOT(openPreferences()));

    connect(mUi->actionShowGrid, SIGNAL(toggled(bool)),
            preferences, SLOT(setShowGrid(bool)));
    connect(mUi->actionShowTileObjectOutlines, SIGNAL(toggled(bool)),
            preferences, SLOT(setShowTileObjectOutlines(bool)));
    connect(mUi->actionShowTileAnimations, SIGNAL(toggled(bool)),
            preferences, SLOT(setShowTileAnimations(bool)));
    connect(mUi->actionSnapToGrid, SIGNAL(toggled(bool)),
            preferences, SLOT(setSnapToGrid(bool)));
    connect(mUi->actionSnapToFineGrid, SIGNAL(toggled(bool)),
            preferences, SLOT(setSnapToFineGrid(bool)));
    connect(mUi->actionHighlightCurrentLayer, SIGNAL(toggled(bool)),
            preferences, SLOT(setHighlightCurrentLayer(bool)));
    connect(mUi->actionZoomIn, SIGNAL(triggered()), SLOT(zoomIn()));
    connect(mUi->actionZoomOut, SIGNAL(triggered()), SLOT(zoomOut()));
    connect(mUi->actionZoomNormal, SIGNAL(triggered()), SLOT(zoomNormal()));

    connect(mUi->actionNewTileset, SIGNAL(triggered()), SLOT(newTileset()));
    connect(mUi->actionAddExternalTileset, SIGNAL(triggered()),
            SLOT(addExternalTileset()));
    connect(mUi->actionResizeMap, SIGNAL(triggered()), SLOT(resizeMap()));
    connect(mUi->actionMapProperties, SIGNAL(triggered()),
            SLOT(editMapProperties()));

    connect(mUi->actionAbout, SIGNAL(triggered()), SLOT(aboutTiled()));

    // Add recent file actions to the recent files menu
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
         mRecentFiles[i] = new QAction(this);
         mUi->menuRecentFiles->insertAction(mUi->actionClearRecentFiles,
                                            mRecentFiles[i]);
         mRecentFiles[i]->setVisible(false);
         connect(mRecentFiles[i], SIGNAL(triggered()),
                 this, SLOT(openRecentFile()));
    }
    mUi->menuRecentFiles->insertSeparator(mUi->actionClearRecentFiles);

    setThemeIcon(mUi->actionNew, "document-new");
    setThemeIcon(mUi->actionOpen, "document-open");
    setThemeIcon(mUi->menuRecentFiles, "document-open-recent");
    setThemeIcon(mUi->actionClearRecentFiles, "edit-clear");
    setThemeIcon(mUi->actionSave, "document-save");
    setThemeIcon(mUi->actionSaveAs, "document-save-as");
    setThemeIcon(mUi->actionClose, "window-close");
    setThemeIcon(mUi->actionQuit, "application-exit");
    setThemeIcon(mUi->actionCut, "edit-cut");
    setThemeIcon(mUi->actionCopy, "edit-copy");
    setThemeIcon(mUi->actionPaste, "edit-paste");
    setThemeIcon(mUi->actionDelete, "edit-delete");
    setThemeIcon(redoAction, "edit-redo");
    setThemeIcon(undoAction, "edit-undo");
    setThemeIcon(mUi->actionZoomIn, "zoom-in");
    setThemeIcon(mUi->actionZoomOut, "zoom-out");
    setThemeIcon(mUi->actionZoomNormal, "zoom-original");
    setThemeIcon(mUi->actionNewTileset, "document-new");
    setThemeIcon(mUi->actionResizeMap, "document-page-setup");
    setThemeIcon(mUi->actionMapProperties, "document-properties");
    setThemeIcon(mUi->actionAbout, "help-about");

    mStampBrush = new StampBrush(this);
    mTerrainBrush = new TerrainBrush(this);
    mBucketFillTool = new BucketFillTool(this);
    mSelectAreaTool = new RTBSelectAreaTool(this);
    mUi->mainToolBar->addAction(mToolManager->registerTool(mSelectAreaTool));
    mInsertTool = new RTBInsertTool(this);
    mUi->mainToolBar->addAction(mToolManager->registerTool(mInsertTool));

    connect(mInsertTool, SIGNAL(cancelInsert()),
            mToolManager, SLOT(selectDefaultTool()));

    connect(mStampBrush, SIGNAL(stampCaptured(TileStamp)),
            this, SLOT(setStamp(TileStamp)));

    mObjectSelectionTool = new ObjectSelectionTool(this);

    QToolBar *toolBar = mUi->toolsToolBar;
    toolBar->addAction(mToolManager->registerTool(mStampBrush));
    toolBar->addAction(mToolManager->registerTool(mBucketFillTool));
    toolBar->addAction(mToolManager->registerTool(new Eraser(this)));
    toolBar->addAction(mToolManager->registerTool(new TileSelectionTool(this)));
    toolBar->addAction(mToolManager->registerTool(mObjectSelectionTool));

    connect(mValidatorDock, SIGNAL(validatorItemClicked(MapObject*))
                                   , this, SLOT(activateObjectSelectionTool(MapObject*)));
    connect(mValidatorDock, SIGNAL(validatorItemClicked(MapObject*))
                                   , mObjectSelectionTool, SLOT(selectObject(MapObject*)));
    connect(mValidatorDock, SIGNAL(validatorItemClicked(int))
                                   , mToolManager, SLOT(highlightToolbarAction(int)));
    connect(mValidatorDock, SIGNAL(validatorItemClicked(int))
                                   , mToolManager, SLOT(activateToolbarAction(int)));
    connect(mValidator, SIGNAL(highlightToolbarAction(int))
                                   , mToolManager, SLOT(highlightToolbarAction(int)));

    // create layer shortcuts
    mFloorLayerShortcut = new QShortcut(QKeySequence(Qt::Key_1), this);
    mOrbLayerShortcut = new QShortcut(QKeySequence(Qt::Key_3), this);
    mObjectLayerShortcut = new QShortcut(QKeySequence(Qt::Key_2), this);
    // create interval speed shortcuts
    mIntervalSpeedShortcut1 = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_1), this);
    mIntervalSpeedShortcut2 = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_2), this);
    mIntervalSpeedShortcut3 = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_3), this);
    mIntervalSpeedShortcut4 = new QShortcut(QKeySequence(Qt::ALT + Qt::Key_4), this);
    // create interval offset shortcuts
    mIntervalOffsetShortcut1 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_1), this);
    mIntervalOffsetShortcut2 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_2), this);
    mIntervalOffsetShortcut3 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_3), this);
    mIntervalOffsetShortcut4 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_4), this);
    mIntervalOffsetShortcut5 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_5), this);
    mIntervalOffsetShortcut6 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_6), this);
    mIntervalOffsetShortcut7 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_7), this);
    mIntervalOffsetShortcut8 = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_8), this);
    // create change layer shortcuts
    mChangeLayerShortcut = new QShortcut(QKeySequence(Qt::Key_Tab), this);
    mChangeLayerBackShortcut = new QShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Tab), this);

    mTileSelectionManager->setStampBrush(mStampBrush);
    mTileSelectionManager->setBucketFillTool(mBucketFillTool);

    mTileSelectionManager->setSeparatorAction(toolBar->addSeparator());
    mToolManager->setSeparatorAction(toolBar->addSeparator());

    // RTB: create floor tools
    toolBar->addAction(mTileSelectionManager->registerTile(new RTBTileButton(this, RTBMapSettings::Floor, RTBMapSettings::FloorID)));
    toolBar->addAction(mTileSelectionManager->registerTile(new RTBTileButton(this, RTBMapSettings::FloorTrap, RTBMapSettings::FloorID)));
    toolBar->addAction(mTileSelectionManager->registerTile(new RTBTileButton(this, RTBMapSettings::Barrier, RTBMapSettings::FloorID)));
    toolBar->addAction(mTileSelectionManager->registerTile(new RTBTileButton(this, RTBMapSettings::HiddenFloor, RTBMapSettings::FloorID)));
    toolBar->addAction(mTileSelectionManager->registerTile(new RTBTileButton(this, RTBMapSettings::SpeedpadRight, RTBMapSettings::FloorID)));
    toolBar->addAction(mTileSelectionManager->registerTile(new RTBTileButton(this, RTBMapSettings::SpeedpadLeft, RTBMapSettings::FloorID)));
    toolBar->addAction(mTileSelectionManager->registerTile(new RTBTileButton(this, RTBMapSettings::SpeedpadUp, RTBMapSettings::FloorID)));
    toolBar->addAction(mTileSelectionManager->registerTile(new RTBTileButton(this, RTBMapSettings::SpeedpadDown, RTBMapSettings::FloorID)));
    toolBar->addAction(mTileSelectionManager->registerTile(new RTBTileButton(this, RTBMapSettings::Jumppad, RTBMapSettings::FloorID)));
    toolBar->addAction(mTileSelectionManager->registerTile(new RTBTileButton(this, RTBMapSettings::WallBlock, RTBMapSettings::FloorID)));

    // RTB: create orb tools
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::PointOrb, RTBMapSettings::OrbObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::CheckpointOrb, RTBMapSettings::OrbObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::HealthOrb, RTBMapSettings::OrbObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::KeyOrb, RTBMapSettings::OrbObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::FakeOrb, RTBMapSettings::OrbObjectID)));

    // RTB: create object tools
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::StartLocation, RTBMapSettings::ObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::FinishHole, RTBMapSettings::ObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::FloorText, RTBMapSettings::ObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::CameraTrigger, RTBMapSettings::ObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::Target, RTBMapSettings::ObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::CustomFloorTrap, RTBMapSettings::ObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::LaserBeamLeft, RTBMapSettings::ObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::LaserBeamBottom, RTBMapSettings::ObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::LaserBeamTop, RTBMapSettings::ObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::LaserBeamRight, RTBMapSettings::ObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::Button, RTBMapSettings::ObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::MovingFloorTrapSpawner, RTBMapSettings::ObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::Teleporter, RTBMapSettings::ObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::NPCBallSpawner, RTBMapSettings::ObjectID)));
    toolBar->addAction(mToolManager->registerTool(new RTBCreateObjectTool(this, RTBMapObject::ProjectileTurret, RTBMapSettings::ObjectID)));


    connect(mTileSelectionManager, SIGNAL(selectedTileChanged(AbstractTool *)),
            mToolManager, SLOT(selectDefaultTileTool()));


    mDocumentManager->setSelectedTool(mToolManager->selectedTool());
    connect(mToolManager, SIGNAL(selectedToolChanged(AbstractTool*)),
            mDocumentManager, SLOT(setSelectedTool(AbstractTool*)));

    statusBar()->addWidget(mStatusInfoLabel);
    connect(mToolManager, SIGNAL(statusInfoChanged(QString)),
            this, SLOT(updateStatusInfoLabel(QString)));
    statusBar()->addWidget(mCurrentLayerLabel);

    // Add the 'Views and Toolbars' submenu. This needs to happen after all
    // the dock widgets and toolbars have been added to the main window.
    mViewsAndToolbarsMenu = new QAction(tr("Views and Toolbars"), this);
    QMenu *popupMenu = createPopupMenu();
    popupMenu->setParent(this);
    mViewsAndToolbarsMenu->setMenu(popupMenu);
    mUi->menuView->insertAction(mUi->actionShowGrid, mViewsAndToolbarsMenu);
    mUi->menuView->insertSeparator(mUi->actionShowGrid);

    connect(ClipboardManager::instance(), SIGNAL(hasMapChanged()), SLOT(updateActions()));

    connect(mDocumentManager, SIGNAL(currentDocumentChanged(MapDocument*)),
            SLOT(mapDocumentChanged(MapDocument*)));
    connect(mDocumentManager, SIGNAL(documentCloseRequested(int)),
            this, SLOT(closeMapDocument(int)));
    connect(mDocumentManager, SIGNAL(reloadError(QString)),
            this, SLOT(reloadError(QString)));

    QShortcut *switchToLeftDocument = new QShortcut(tr("Alt+Left"), this);
    connect(switchToLeftDocument, SIGNAL(activated()),
            mDocumentManager, SLOT(switchToLeftDocument()));
    QShortcut *switchToLeftDocument1 = new QShortcut(tr("Ctrl+Shift+Tab"), this);
    connect(switchToLeftDocument1, SIGNAL(activated()),
            mDocumentManager, SLOT(switchToLeftDocument()));

    QShortcut *switchToRightDocument = new QShortcut(tr("Alt+Right"), this);
    connect(switchToRightDocument, SIGNAL(activated()),
            mDocumentManager, SLOT(switchToRightDocument()));
    QShortcut *switchToRightDocument1 = new QShortcut(tr("Ctrl+Tab"), this);
    connect(switchToRightDocument1, SIGNAL(activated()),
            mDocumentManager, SLOT(switchToRightDocument()));

    QShortcut *copyPositionShortcut = new QShortcut(tr("Alt+C"), this);
    connect(copyPositionShortcut, SIGNAL(activated()),
            mActionHandler, SLOT(copyPosition()));

#if defined(Q_OS_OSX) && QT_VERSION >= 0x050000
    // This works around the problem that the shortcut for the Delete menu action
    // is not working on OS X for whatever reason.
    foreach (QKeySequence key, QKeySequence::keyBindings(QKeySequence::Delete))
        new QShortcut(key, this, SLOT(delete_()));
#endif

    updateActions();
    readSettings();

    mTutorial = new RTBTutorial(mTutorialDock, this);
    connect(mTutorial, SIGNAL(highlightSection(int)),
                this, SLOT(highlightSection(int)));

}

MainWindow::~MainWindow()
{
    mDocumentManager->closeAllDocuments();

    delete mTileStampManager;
    delete mTutorial;
    delete mValidator;
    delete mValidatorDock;
    delete mTutorialDock;

    TilesetManager::deleteInstance();
    DocumentManager::deleteInstance();
    Preferences::deleteInstance();
    LanguageManager::deleteInstance();
    PluginManager::deleteInstance();
    ClipboardManager::deleteInstance();
    RTBCore::deleteInstance();

    delete mUi;
}

void MainWindow::commitData(QSessionManager &manager)
{
    // Play nice with session management and cancel shutdown process when user
    // requests this
    if (manager.allowsInteraction())
        if (!confirmAllSave())
            manager.cancel();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeSettings();

    if (confirmAllSave())
        event->accept();
    else
        event->ignore();
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange:
        mUi->retranslateUi(this);
        retranslateUi();
        break;
    default:
        break;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat())
        if (MapView *mapView = mDocumentManager->currentMapView())
            mapView->setHandScrolling(true);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space && !event->isAutoRepeat())
        if (MapView *mapView = mDocumentManager->currentMapView())
            mapView->setHandScrolling(false);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    const QList<QUrl> urls = e->mimeData()->urls();
    if (!urls.isEmpty() && !urls.at(0).toLocalFile().isEmpty())
        e->accept();
}

void MainWindow::dropEvent(QDropEvent *e)
{
    foreach (const QUrl &url, e->mimeData()->urls())
        openFile(url.toLocalFile());
}

void MainWindow::newMap()
{
    NewMapDialog newMapDialog(this);
    MapDocument *mapDocument = newMapDialog.createMap();

    if (!mapDocument)
        return;

    mDocumentManager->addDocument(mapDocument);
}

bool MainWindow::openFile(const QString &fileName,
                          MapReaderInterface *mapReader)
{
    if (fileName.isEmpty())
        return false;

    // Select existing document if this file is already open
    int documentIndex = mDocumentManager->findDocument(fileName);
    if (documentIndex != -1) {
        mDocumentManager->switchToDocument(documentIndex);
        return true;
    }

    QString error;
    MapDocument *mapDocument = MapDocument::load(fileName, mapReader, &error);
    if (!mapDocument) {
        QMessageBox::critical(this, tr("Error Opening Map"), error);
        return false;
    }

    mDocumentManager->addDocument(mapDocument);
    setRecentFile(fileName);

    // validate the map
    mapDocument->map()->rtbMap()->setHasError(mValidator->validate());

    return true;
}

bool MainWindow::openFile(const QString &fileName)
{
    return openFile(fileName, 0);
}

void MainWindow::openLastFiles()
{
    mSettings->beginGroup(QLatin1String("recentFiles"));

    QStringList lastOpenFiles = mSettings->value(
                QLatin1String("lastOpenFiles")).toStringList();
    QVariant openCountVariant = mSettings->value(
                QLatin1String("recentOpenedFiles"));

    // Backwards compatibility mode
    if (openCountVariant.isValid()) {
        const QStringList recentFiles = mSettings->value(
                    QLatin1String("fileNames")).toStringList();
        int openCount = qMin(openCountVariant.toInt(), recentFiles.size());
        for (; openCount; --openCount)
            lastOpenFiles.append(recentFiles.at(openCount - 1));
        mSettings->remove(QLatin1String("recentOpenedFiles"));
    }

    QStringList mapScales = mSettings->value(
                QLatin1String("mapScale")).toStringList();
    QStringList scrollX = mSettings->value(
                QLatin1String("scrollX")).toStringList();
    QStringList scrollY = mSettings->value(
                QLatin1String("scrollY")).toStringList();
    QStringList selectedLayer = mSettings->value(
                QLatin1String("selectedLayer")).toStringList();

    for (int i = 0; i < lastOpenFiles.size(); i++) {
        if (!(i < mapScales.size()))
            continue;
        if (!(i < scrollX.size()))
            continue;
        if (!(i < scrollY.size()))
            continue;
        if (!(i < selectedLayer.size()))
            continue;

        if (openFile(lastOpenFiles.at(i))) {
            MapView *mapView = mDocumentManager->currentMapView();

            // Restore camera to the previous position
            qreal scale = mapScales.at(i).toDouble();
            if (scale > 0)
                mapView->zoomable()->setScale(scale);

            const int hor = scrollX.at(i).toInt();
            const int ver = scrollY.at(i).toInt();
            mapView->horizontalScrollBar()->setSliderPosition(hor);
            mapView->verticalScrollBar()->setSliderPosition(ver);

            int layer = selectedLayer.at(i).toInt();
            if (layer > 0 && layer < mMapDocument->map()->layerCount())
                mMapDocument->setCurrentLayerIndex(layer);
        }
    }
    QString lastActiveDocument =
            mSettings->value(QLatin1String("lastActive")).toString();
    int documentIndex = mDocumentManager->findDocument(lastActiveDocument);
    if (documentIndex != -1)
        mDocumentManager->switchToDocument(documentIndex);

    mSettings->endGroup();
}

void MainWindow::openFile()
{
    QString filter = tr("Json files (*.json)");

    const PluginManager *pm = PluginManager::instance();
    QList<MapReaderInterface*> readers = pm->interfaces<MapReaderInterface>();

    QString selectedFilter = tr("Json files (*.json)");
    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open Map"),
                                                    fileDialogStartLocation(),
                                                    filter, &selectedFilter);
    if (fileNames.isEmpty())
        return;

    // When a particular filter was selected, use the associated reader
    MapReaderInterface *mapReader = 0;
    foreach (MapReaderInterface *reader, readers) {
        if (reader->nameFilters().contains(selectedFilter))
            mapReader = reader;
    }

    mSettings->setValue(QLatin1String("lastUsedOpenFilter"), selectedFilter);
    foreach (const QString &fileName, fileNames)
        openFile(fileName, mapReader);
}

bool MainWindow::saveFile(const QString &fileName)
{
    if (!mMapDocument)
        return false;

    if (fileName.isEmpty())
        return false;

    mToolManager->resetToolbarActionIcons();
    mMapDocument->map()->rtbMap()->setHasError(mValidator->validate());

    QString error;
    if (!mMapDocument->save(fileName, &error)) {
        QMessageBox::critical(this, tr("Error Saving Map"), error);
        return false;
    }

    setRecentFile(fileName);
    return true;
}

bool MainWindow::saveFile()
{
    if (!mMapDocument)
        return false;

    const QString currentFileName = mMapDocument->fileName();

    if (!saveFile(currentFileName))
        return saveFileAsJSON();

    return true;
}

bool MainWindow::saveFileAs()
{
    const QString tmxfilter = tr("Tiled map files (*.tmx)");
    QString filter = QString(tmxfilter);
    PluginManager *pm = PluginManager::instance();
    foreach (const Plugin &plugin, pm->plugins()) {
        const MapWriterInterface *writer = qobject_cast<MapWriterInterface*>
                (plugin.instance);
        const MapReaderInterface *reader = qobject_cast<MapReaderInterface*>
                (plugin.instance);
        if (writer && reader) {
            foreach (const QString &str, writer->nameFilters()) {
                if (!str.isEmpty()) {
                    filter += QLatin1String(";;");
                    filter += str;
                }
            }
        }
    }

    QString selectedFilter;
    if (mMapDocument)
        selectedFilter = mMapDocument->writerPluginFileName();

    if (selectedFilter.isEmpty())
        selectedFilter = tmxfilter;

    QString suggestedFileName;
    if (mMapDocument && !mMapDocument->fileName().isEmpty()) {
        suggestedFileName = mMapDocument->fileName();
    } else {
        suggestedFileName = fileDialogStartLocation();
        suggestedFileName += QLatin1Char('/');
        suggestedFileName += tr("untitled.tmx");
    }

    const QString fileName =
            QFileDialog::getSaveFileName(this, QString(), suggestedFileName,
                                         filter, &selectedFilter);

    if (fileName.isEmpty())
        return false;

    QString writerPluginFilename;
    if (const Plugin *p = pm->pluginByNameFilter(selectedFilter))
        writerPluginFilename = p->fileName;

    mMapDocument->setWriterPluginFileName(writerPluginFilename);

    return saveFile(fileName);
}

void MainWindow::saveAll()
{
    foreach (MapDocument *mapDoc, mDocumentManager->documents()) {
        if (!mapDoc->isModified())
            continue;

        QString fileName(mapDoc->fileName());
        QString error;

        if (fileName.isEmpty()) {
            mDocumentManager->switchToDocument(mapDoc);
            if (!saveFileAs())
                return;
        } else if (!mapDoc->save(fileName, &error)) {
            mDocumentManager->switchToDocument(mapDoc);
            QMessageBox::critical(this, tr("Error Saving Map"), error);
            return;
        }

        setRecentFile(fileName);
    }
}

bool MainWindow::confirmSave(MapDocument *mapDocument)
{
    if (!mapDocument || !mapDocument->isModified())
        return true;

    mDocumentManager->switchToDocument(mapDocument);

    int ret = QMessageBox::warning(
            this, tr("Unsaved Changes"),
            tr("There are unsaved changes. Do you want to save now?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch (ret) {
    case QMessageBox::Save:    return saveFile();
    case QMessageBox::Discard: return true;
    case QMessageBox::Cancel:
    default:
        return false;
    }
}

bool MainWindow::confirmAllSave()
{
    for (int i = 0; i < mDocumentManager->documentCount(); ++i) {
        if (!confirmSave(mDocumentManager->documents().at(i)))
            return false;
    }

    return true;
}

void MainWindow::export_()
{
    if (!mMapDocument)
        return;

    QString exportFileName = mMapDocument->lastExportFileName();
    QString exportPluginFileName = mMapDocument->exportPluginFileName();
    TmxMapWriter mapWriter;

    if (!exportFileName.isEmpty()) {
        MapWriterInterface *writer = 0;

        if (exportPluginFileName.isEmpty()) {
            writer = &mapWriter;
        } else {
            PluginManager *pm = PluginManager::instance();
            if (const Plugin *plugin = pm->pluginByFileName(exportPluginFileName))
                writer = qobject_cast<MapWriterInterface*>(plugin->instance);
        }

        if (writer) {
            if (writer->write(mMapDocument->map(), exportFileName)) {
                statusBar()->showMessage(tr("Exported to %1").arg(exportFileName),
                                         3000);
                return;
            }

            QMessageBox::critical(this, tr("Error Exporting Map"),
                                  writer->errorString());
        }
    }

    // fall back when no successful export happened
    exportAs();
}

void MainWindow::exportAs()
{
    if (!mMapDocument)
        return;

    PluginManager *pm = PluginManager::instance();
    QList<MapWriterInterface*> writers = pm->interfaces<MapWriterInterface>();
    QString filter = tr("All Files (*)");
    foreach (const MapWriterInterface *writer, writers) {
        foreach (const QString &str, writer->nameFilters()) {
            if (!str.isEmpty()) {
                filter += QLatin1String(";;");
                filter += str;
            }
        }
    }

    Preferences *pref = Preferences::instance();

    QString selectedFilter =
            mSettings->value(QLatin1String("lastUsedExportFilter")).toString();
    QString suggestedFilename = mMapDocument->lastExportFileName();

    if (suggestedFilename.isEmpty()) {
        QFileInfo baseNameInfo = QFileInfo(mMapDocument->fileName());
        QString baseName = baseNameInfo.baseName();

        QRegExp extensionFinder(QLatin1String("\\(\\*\\.([^\\)\\s]*)"));
        extensionFinder.indexIn(selectedFilter);
        const QString extension = extensionFinder.cap(1);

        QString lastExportedFilePath = pref->lastPath(Preferences::ExportedFile);

        suggestedFilename = lastExportedFilePath
                + QLatin1String("/") + baseName
                + QLatin1Char('.') + extension;
    }

    // No need to confirm overwrite here since it'll be prompted below
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export As..."),
                                                    suggestedFilename,
                                                    filter, &selectedFilter,
                                                    QFileDialog::DontConfirmOverwrite);
    if (fileName.isEmpty())
        return;

    MapWriterInterface *chosenWriter = 0;

    // If a specific filter was selected, use that writer
    foreach (MapWriterInterface *writer, writers)
        if (writer->nameFilters().contains(selectedFilter))
            chosenWriter = writer;

    // If not, try to find the file extension among the name filters
    QString suffix = QFileInfo(fileName).completeSuffix();
    if (!chosenWriter && !suffix.isEmpty()) {
        suffix.prepend(QLatin1String("*."));

        foreach (MapWriterInterface *writer, writers) {
            if (!writer->nameFilters().filter(suffix,
                                              Qt::CaseInsensitive).isEmpty()) {
                if (chosenWriter) {
                    QMessageBox::warning(this, tr("Non-unique file extension"),
                                         tr("Non-unique file extension.\n"
                                            "Please select specific format."));
                    exportAs();
                    return;
                } else {
                    chosenWriter = writer;
                }
            }
        }
    }

    // Also support exporting to the TMX map format when requested
    TmxMapWriter tmxMapWriter;
    if (!chosenWriter && fileName.endsWith(QLatin1String(".tmx"),
                                           Qt::CaseInsensitive))
        chosenWriter = &tmxMapWriter;

    if (!chosenWriter) {
        QMessageBox::critical(this, tr("Unknown File Format"),
                              tr("The given filename does not have any known "
                                 "file extension."));
        return;
    }

    // Check if writer will overwrite existing files here because some writers
    // could save to multiple files at the same time. For example CSV saves
    // each layer into a separate file.
    QStringList outputFiles = chosenWriter->outputFiles(mMapDocument->map(),
                                                        fileName);
    if (outputFiles.size() > 0) {
        // Check if any output file already exists
        QString message =
                tr("Some export files already exist:") + QLatin1String("\n\n");

        bool overwriteHappens = false;

        foreach (const QString &outputFile, outputFiles) {
            if (QFile::exists(outputFile)) {
                overwriteHappens = true;
                message += outputFile + QLatin1Char('\n');
            }
        }
        message += QLatin1Char('\n') + tr("Do you want to replace them?");

        // If overwrite happens, warn the user and get confirmation before executing the writer
        if (overwriteHappens) {
            const QMessageBox::StandardButton reply = QMessageBox::warning(
                this,
                tr("Overwrite Files"),
                message,
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No);

            if (reply != QMessageBox::Yes)
                return;
        }
    }

    pref->setLastPath(Preferences::ExportedFile, QFileInfo(fileName).path());
    mSettings->setValue(QLatin1String("lastUsedExportFilter"), selectedFilter);

    if (!chosenWriter->write(mMapDocument->map(), fileName)) {
        QMessageBox::critical(this, tr("Error Exporting Map"),
                              chosenWriter->errorString());
    } else {
        // Remember export parameters, so subsequent exports can be done faster
        mMapDocument->setLastExportFileName(fileName);
        QString exportPluginFileName;
        if (const Plugin *plugin = pm->plugin(chosenWriter))
            exportPluginFileName = plugin->fileName;
        mMapDocument->setExportPluginFileName(exportPluginFileName);
    }
}

void MainWindow::exportAsImage()
{
    if (!mMapDocument)
        return;

    MapView *mapView = mDocumentManager->currentMapView();
    ExportAsImageDialog dialog(mMapDocument,
                               mMapDocument->fileName(),
                               mapView->zoomable()->scale(),
                               this);
    dialog.exec();
}

void MainWindow::reload()
{
    if (confirmSave(mDocumentManager->currentDocument()))
        mDocumentManager->reloadCurrentDocument();
}

void MainWindow::closeFile()
{
    if (confirmSave(mDocumentManager->currentDocument()))
        mDocumentManager->closeCurrentDocument();
}

void MainWindow::closeAllFiles()
{
    if (confirmAllSave())
        mDocumentManager->closeAllDocuments();
}

void MainWindow::cut()
{
    if (!mMapDocument)
        return;

    Layer *currentLayer = mMapDocument->currentLayer();
    if (!currentLayer)
        return;

    TileLayer *tileLayer = dynamic_cast<TileLayer*>(currentLayer);
    const QRegion &selectedArea = mMapDocument->selectedArea();
    const QList<MapObject*> &selectedObjects = mMapDocument->selectedObjects();

    copy(true);

    QUndoStack *stack = mMapDocument->undoStack();
    stack->beginMacro(tr("Cut"));

    if(mSelectAreaTool->isActive())
    {
        // delete floor tiles
        if (tileLayer && !selectedArea.isEmpty())
            stack->push(new EraseTiles(mMapDocument, tileLayer, selectedArea));

        // delete all objects in the selected area objects
        QList<MapObject*> objects = mMapDocument->map()->objectGroups().at(0)->objects();
        QList<MapObject*> orbs = mMapDocument->map()->objectGroups().at(1)->objects();

        // convert the grid area to a normal rect
        QPointF topLeft = selectedArea.boundingRect().topLeft() * 32;
        QPointF bottomRight = selectedArea.boundingRect().bottomRight() * 32;
        bottomRight.setX(bottomRight.x() + 32);
        bottomRight.setY(bottomRight.y() + 32);
        QRectF area(topLeft, bottomRight);

        for(MapObject *obj : objects)
        {
            if(area.contains(obj->position().toPoint()))
                stack->push(new RemoveMapObject(mMapDocument, obj));
        }

        for(MapObject *obj : orbs)
        {
            if(area.contains(obj->boundsUseTile().center().toPoint()))
                stack->push(new RemoveMapObject(mMapDocument, obj));
        }
    }
    else
    {
        if (tileLayer && !selectedArea.isEmpty()) {
            stack->push(new EraseTiles(mMapDocument, tileLayer, selectedArea));
        } else if (!selectedObjects.isEmpty()) {
            foreach (MapObject *mapObject, selectedObjects)
                stack->push(new RemoveMapObject(mMapDocument, mapObject));
        }
    }

    mActionHandler->selectNone();

    stack->endMacro();
}

void MainWindow::copy(bool isCut)
{
    if (!mMapDocument)
        return;

    ClipboardManager *clipboardManager = ClipboardManager::instance();
    clipboardManager->setIsCut(isCut);

    if(mSelectAreaTool->isActive())
        clipboardManager->copySelectionAllLayers(mMapDocument);
    else
        clipboardManager->copySelection(mMapDocument);
}

void MainWindow::paste()
{
    if (!mMapDocument)
        return;

    Layer *currentLayer = mMapDocument->currentLayer();
    if (!currentLayer)
        return;

    ClipboardManager *clipboardManager = ClipboardManager::instance();
    QScopedPointer<Map> map(clipboardManager->map());
    if (!map)
        return;

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->addReferences(map->tilesets());
    mMapDocument->unifyTilesets(map.data());

    // We can currently only handle maps with a single layer
    if (map->layerCount() == 1)
    {
        Layer *layer = map->layerAt(0);

        if (layer->isTileLayer()) {
            // Reset selection and paste into the stamp brush
            mActionHandler->selectNone();
            Map *stamp = map.take(); // TileStamp will take ownership
            setStamp(TileStamp(stamp));
            tilesetManager->removeReferences(stamp->tilesets());
            mToolManager->selectTool(mStampBrush);
        } else if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
            const MapView *view = mDocumentManager->currentMapView();
            clipboardManager->pasteObjectGroup(objectGroup, mMapDocument, view);
        }
    }
    else
    {
        TileLayer *floorLayer = map->layerAt(RTBMapSettings::FloorID)->asTileLayer();

        mActionHandler->selectNone();
        Map *stamp = new Map(map->orientation(),
                             floorLayer->width(),
                             floorLayer->height(),
                             map->tileWidth(),
                             map->tileHeight());
        stamp->addLayer(floorLayer->clone());
        mInsertTool->setStamp(TileStamp(stamp));

        mToolManager->selectTool(mInsertTool);
    }

    if (map)
        tilesetManager->removeReferences(map->tilesets());
}

void MainWindow::delete_()
{
    if (!mMapDocument)
        return;

    Layer *currentLayer = mMapDocument->currentLayer();
    if (!currentLayer)
        return;

    if(!mSelectAreaTool->isActive())
    {
        TileLayer *tileLayer = dynamic_cast<TileLayer*>(currentLayer);
        const QRegion &selectedArea = mMapDocument->selectedArea();
        const QList<MapObject*> &selectedObjects = mMapDocument->selectedObjects();

        QUndoStack *undoStack = mMapDocument->undoStack();
        undoStack->beginMacro(tr("Delete"));

        if (tileLayer && !selectedArea.isEmpty()) {
            undoStack->push(new EraseTiles(mMapDocument, tileLayer, selectedArea));
        } else if (!selectedObjects.isEmpty()) {
            foreach (MapObject *mapObject, selectedObjects)
                undoStack->push(new RemoveMapObject(mMapDocument, mapObject));
        }

        mActionHandler->selectNone();
        undoStack->endMacro();
    }
    else
    {
        mSelectAreaTool->deleteArea();
    }
}

void MainWindow::openPreferences()
{
    PreferencesDialog preferencesDialog(this);
    preferencesDialog.exec();
}

void MainWindow::zoomIn()
{
    if (MapView *mapView = mDocumentManager->currentMapView())
        mapView->zoomable()->zoomIn();
}

void MainWindow::zoomOut()
{
    if (MapView *mapView = mDocumentManager->currentMapView())
        mapView->zoomable()->zoomOut();
}

void MainWindow::zoomNormal()
{
    if (MapView *mapView = mDocumentManager->currentMapView())
        mapView->zoomable()->resetZoom();
}

bool MainWindow::newTileset(const QString &path)
{
    if (!mMapDocument)
        return false;

    Map *map = mMapDocument->map();
    Preferences *prefs = Preferences::instance();

    const QString startLocation = path.isEmpty()
            ? QFileInfo(prefs->lastPath(Preferences::ImageFile)).absolutePath()
            : path;

    NewTilesetDialog newTileset(startLocation, this);
    newTileset.setTileWidth(map->tileWidth());
    newTileset.setTileHeight(map->tileHeight());

    if (SharedTileset tileset = newTileset.createTileset()) {
        mMapDocument->undoStack()->push(new AddTileset(mMapDocument, tileset));
        prefs->setLastPath(Preferences::ImageFile, tileset->imageSource());
        return true;
    }
    return false;
}

void MainWindow::newTilesets(const QStringList &paths)
{
    foreach (const QString &path, paths)
        if (!newTileset(path))
            return;
}

void MainWindow::reloadTilesets()
{
    Map *map = mMapDocument->map();
    if (!map)
        return;

    TilesetManager *tilesetManager = TilesetManager::instance();
    QVector<SharedTileset> tilesets = map->tilesets();
    for (SharedTileset &tileset : tilesets)
        tilesetManager->forceTilesetReload(tileset);
}

void MainWindow::addExternalTileset()
{
    if (!mMapDocument)
        return;

    Preferences *prefs = Preferences::instance();
    QString start = prefs->lastPath(Preferences::ExternalTileset);

    const QStringList fileNames =
            QFileDialog::getOpenFileNames(this, tr("Add External Tileset(s)"),
                                          start,
                                          tr("Tiled tileset files (*.tsx)"));

    if (fileNames.isEmpty())
        return;

    prefs->setLastPath(Preferences::ExternalTileset,
                       QFileInfo(fileNames.back()).path());

    QVector<SharedTileset> tilesets;

    foreach (QString fileName, fileNames) {
        TmxMapReader reader;
        if (SharedTileset tileset = reader.readTileset(fileName)) {
            tilesets.append(tileset);
        } else if (fileNames.size() == 1) {
            QMessageBox::critical(this, tr("Error Reading Tileset"),
                                  reader.errorString());
            return;
        } else {
            int result;

            result = QMessageBox::warning(this, tr("Error Reading Tileset"),
                                          tr("%1: %2").arg(fileName, reader.errorString()),
                                          QMessageBox::Abort | QMessageBox::Ignore,
                                          QMessageBox::Ignore);

            if (result == QMessageBox::Abort)
                return;
        }
    }

    QUndoStack *undoStack = mMapDocument->undoStack();
    undoStack->beginMacro(tr("Add %n Tileset(s)", "", tilesets.size()));
    foreach (const SharedTileset &tileset, tilesets)
        undoStack->push(new AddTileset(mMapDocument, tileset));
    undoStack->endMacro();
}

void MainWindow::resizeMap()
{
    if (!mMapDocument)
        return;

    Map *map = mMapDocument->map();

    ResizeDialog resizeDialog(this);
    resizeDialog.setOldSize(map->size());

    if (resizeDialog.exec()) {
        const QSize &newSize = resizeDialog.newSize();
        const QPoint &offset = resizeDialog.offset();
        if (newSize != map->size() || !offset.isNull())
            mMapDocument->resizeMap(newSize, offset);
    }
}

void MainWindow::offsetMap()
{
    if (!mMapDocument)
        return;

    OffsetMapDialog offsetDialog(mMapDocument, this);
    if (offsetDialog.exec()) {
        const QList<int> layerIndexes = offsetDialog.affectedLayerIndexes();
        if (layerIndexes.empty())
            return;

        mMapDocument->offsetMap(layerIndexes,
                                offsetDialog.offset(),
                                offsetDialog.affectedBoundingRect(),
                                offsetDialog.wrapX(),
                                offsetDialog.wrapY());
    }
}

void MainWindow::editMapProperties()
{
    if (!mMapDocument)
        return;

    mMapDocument->setCurrentObject(mMapDocument->map());
    mMapDocument->emitEditCurrentObject();
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        openFile(action->data().toString());
}

QStringList MainWindow::recentFiles() const
{
    QVariant v = mSettings->value(QLatin1String("recentFiles/fileNames"));
    return v.toStringList();
}

QString MainWindow::fileDialogStartLocation() const
{
    QStringList files = recentFiles();
    return (!files.isEmpty()) ? QFileInfo(files.first()).path() : QString();
}

/**
 * Adds the given file to the recent files list.
 */
void MainWindow::setRecentFile(const QString &fileName)
{
    // Remember the file by its canonical file path
    const QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    if (canonicalFilePath.isEmpty())
        return;

    QStringList files = recentFiles();
    files.removeAll(canonicalFilePath);
    files.prepend(canonicalFilePath);
    while (files.size() > MaxRecentFiles)
        files.removeLast();

    mSettings->beginGroup(QLatin1String("recentFiles"));
    mSettings->setValue(QLatin1String("fileNames"), files);
    mSettings->endGroup();
    updateRecentFiles();
}

void MainWindow::clearRecentFiles()
{
    mSettings->beginGroup(QLatin1String("recentFiles"));
    mSettings->setValue(QLatin1String("fileNames"), QStringList());
    mSettings->endGroup();
    updateRecentFiles();
}

/**
 * Updates the recent files menu.
 */
void MainWindow::updateRecentFiles()
{
    QStringList files = recentFiles();
    const int numRecentFiles = qMin(files.size(), (int) MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i)
    {
        mRecentFiles[i]->setText(QFileInfo(files[i]).fileName());
        mRecentFiles[i]->setData(files[i]);
        mRecentFiles[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
    {
        mRecentFiles[j]->setVisible(false);
    }
    mUi->menuRecentFiles->setEnabled(numRecentFiles > 0);
}

void MainWindow::updateActions()
{
    Map *map = 0;
    bool tileLayerSelected = false;
    bool objectsSelected = false;
    QRegion selection;

    if (mMapDocument) {
        Layer *currentLayer = mMapDocument->currentLayer();

        map = mMapDocument->map();
        tileLayerSelected = dynamic_cast<TileLayer*>(currentLayer) != 0;
        objectsSelected = !mMapDocument->selectedObjects().isEmpty();
        selection = mMapDocument->selectedArea();
    }

    const bool canCopy = (tileLayerSelected && !selection.isEmpty())
            || objectsSelected;

    mUi->actionSave->setEnabled(map);
    mUi->actionSaveAs->setEnabled(map);
    mUi->actionSaveAll->setEnabled(map);
    mUi->actionExportAsImage->setEnabled(map);
    mUi->actionReload->setEnabled(map);
    mUi->actionClose->setEnabled(map);
    mUi->actionCloseAll->setEnabled(map);
    mUi->actionCut->setEnabled(canCopy);
    mUi->actionCopy->setEnabled(canCopy);
    mUi->actionPaste->setEnabled(ClipboardManager::instance()->hasMap());
    mUi->actionDelete->setEnabled(canCopy);
    mUi->actionNewTileset->setEnabled(map);
    mUi->actionAddExternalTileset->setEnabled(map);
    mUi->actionResizeMap->setEnabled(map);
    mUi->actionMapProperties->setEnabled(map);

    mShowMapProperties->setEnabled(map);

    updateZoomLabel(); // for the zoom actions

    Layer *layer = mMapDocument ? mMapDocument->currentLayer() : 0;
    mCurrentLayerLabel->setText(tr("Current layer: %1").arg(
                                    layer ? layer->name() : tr("<none>")));
}

void MainWindow::updateZoomLabel()
{
    MapView *mapView = mDocumentManager->currentMapView();

    Zoomable *zoomable = mapView ? mapView->zoomable() : 0;
    const qreal scale = zoomable ? zoomable->scale() : 1;

    mUi->actionZoomIn->setEnabled(zoomable && zoomable->canZoomIn());
    mUi->actionZoomOut->setEnabled(zoomable && zoomable->canZoomOut());
    mUi->actionZoomNormal->setEnabled(scale != 1);

    if (zoomable) {
        mZoomComboBox->setEnabled(true);
    } else {
        int index = mZoomComboBox->findData((qreal)1.0);
        mZoomComboBox->setCurrentIndex(index);
        mZoomComboBox->setEnabled(false);
    }
}

void MainWindow::flip(FlipDirection direction)
{
    if (mStampBrush->isEnabled()) {
        const TileStamp &stamp = mStampBrush->stamp();
        if (!stamp.isEmpty())
            setStamp(stamp.flipped(direction));

    } else if (mMapDocument) {
        mMapDocument->flipSelectedObjects(direction);
    }
}

void MainWindow::rotate(RotateDirection direction)
{
    if (mStampBrush->isEnabled()) {
        const TileStamp &stamp = mStampBrush->stamp();
        if (!stamp.isEmpty())
            setStamp(stamp.rotated(direction));

    } else if (mMapDocument) {
        mMapDocument->rotateSelectedObjects(direction);
    }
}

/**
 * Sets the current stamp, which is used by both the stamp brush and the bucket
 * fill tool.
 */
void MainWindow::setStamp(const TileStamp &stamp)
{
    if (stamp.isEmpty())
        return;

    mStampBrush->setStamp(stamp);
    mBucketFillTool->setStamp(stamp);

    // When selecting a new stamp, it makes sense to switch to a stamp tool
    AbstractTool *selectedTool = mToolManager->selectedTool();
    if (selectedTool != mStampBrush && selectedTool != mBucketFillTool)
        mToolManager->selectTool(mStampBrush);
}

/**
 * Sets the terrain brush.
 */
void MainWindow::setTerrainBrush(const Terrain *terrain)
{
    mTerrainBrush->setTerrain(terrain);

    // When selecting a new terrain, it makes sense to switch to a terrain brush tool
    AbstractTool *selectedTool = mToolManager->selectedTool();
    if (selectedTool != mTerrainBrush)
        mToolManager->selectTool(mTerrainBrush);
}

void MainWindow::updateStatusInfoLabel(const QString &statusInfo)
{
    mStatusInfoLabel->setText(statusInfo);
}

void MainWindow::writeSettings()
{
    mSettings->beginGroup(QLatin1String("mainwindow"));
    mSettings->setValue(QLatin1String("geometry"), saveGeometry());
    mSettings->setValue(QLatin1String("state"), saveState());
    mSettings->endGroup();

    mSettings->beginGroup(QLatin1String("recentFiles"));
    if (MapDocument *document = mDocumentManager->currentDocument())
        mSettings->setValue(QLatin1String("lastActive"), document->fileName());

    QStringList fileList;
    QStringList mapScales;
    QStringList scrollX;
    QStringList scrollY;
    QStringList selectedLayer;
    for (int i = 0; i < mDocumentManager->documentCount(); i++) {
        MapDocument *document = mDocumentManager->documents().at(i);
        MapView *mapView = mDocumentManager->viewForDocument(document);
        fileList.append(document->fileName());
        const int currentLayerIndex = document->currentLayerIndex();

        mapScales.append(QString::number(mapView->zoomable()->scale()));
        scrollX.append(QString::number(
                       mapView->horizontalScrollBar()->sliderPosition()));
        scrollY.append(QString::number(
                       mapView->verticalScrollBar()->sliderPosition()));
        selectedLayer.append(QString::number(currentLayerIndex));
    }
    mSettings->setValue(QLatin1String("lastOpenFiles"), fileList);
    mSettings->setValue(QLatin1String("mapScale"), mapScales);
    mSettings->setValue(QLatin1String("scrollX"), scrollX);
    mSettings->setValue(QLatin1String("scrollY"), scrollY);
    mSettings->setValue(QLatin1String("selectedLayer"), selectedLayer);
    mSettings->endGroup();
}

void MainWindow::readSettings()
{
    mSettings->beginGroup(QLatin1String("mainwindow"));
    QByteArray geom = mSettings->value(QLatin1String("geometry")).toByteArray();
    if (!geom.isEmpty())
        restoreGeometry(geom);
    else
        resize(1200, 700);
    restoreState(mSettings->value(QLatin1String("state"),
                                 QByteArray()).toByteArray());
    mSettings->endGroup();
    updateRecentFiles();
}

void MainWindow::updateWindowTitle()
{
    if (mMapDocument) {
        setWindowTitle(tr("[*]%1").arg(mMapDocument->displayName()));
        setWindowFilePath(mMapDocument->fileName());
        setWindowModified(mMapDocument->isModified());
    } else {
        setWindowTitle(QString());
        setWindowFilePath(QString());
        setWindowModified(false);
    }
}

void MainWindow::becomePatron()
{
    PatreonDialog patreonDialog(this);
    patreonDialog.exec();
}

void MainWindow::aboutTiled()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}

void MainWindow::retranslateUi()
{
    updateWindowTitle();

    mLayerMenu->setTitle(tr("&Layer"));
    mViewsAndToolbarsMenu->setText(tr("Views and Toolbars"));
    mActionHandler->retranslateUi();
    mToolManager->retranslateTools();
    mTutorial->retranslate();
}

void MainWindow::mapDocumentChanged(MapDocument *mapDocument)
{
    if (mMapDocument)
    {
        mMapDocument->disconnect(this);

        // RTB: disconnect shortcuts
        disconnect(mFloorLayerShortcut, SIGNAL(activated()), mMapDocument, SLOT(selectFloorLayer()));
        disconnect(mOrbLayerShortcut, SIGNAL(activated()), mMapDocument, SLOT(selectOrbLayer()));
        disconnect(mObjectLayerShortcut, SIGNAL(activated()), mMapDocument, SLOT(selectObjectLayer()));
        disconnect(mIntervalSpeedShortcut1, SIGNAL(activated()), mMapDocument, SLOT(setIntervalSpeed1()));
        disconnect(mIntervalSpeedShortcut2, SIGNAL(activated()), mMapDocument, SLOT(setIntervalSpeed2()));
        disconnect(mIntervalSpeedShortcut3, SIGNAL(activated()), mMapDocument, SLOT(setIntervalSpeed3()));
        disconnect(mIntervalSpeedShortcut4, SIGNAL(activated()), mMapDocument, SLOT(setIntervalSpeed4()));
        disconnect(mIntervalOffsetShortcut1, SIGNAL(activated()), mMapDocument, SLOT(setIntervalOffset1()));
        disconnect(mIntervalOffsetShortcut2, SIGNAL(activated()), mMapDocument, SLOT(setIntervalOffset2()));
        disconnect(mIntervalOffsetShortcut3, SIGNAL(activated()), mMapDocument, SLOT(setIntervalOffset3()));
        disconnect(mIntervalOffsetShortcut4, SIGNAL(activated()), mMapDocument, SLOT(setIntervalOffset4()));
        disconnect(mIntervalOffsetShortcut5, SIGNAL(activated()), mMapDocument, SLOT(setIntervalOffset5()));
        disconnect(mIntervalOffsetShortcut6, SIGNAL(activated()), mMapDocument, SLOT(setIntervalOffset6()));
        disconnect(mIntervalOffsetShortcut7, SIGNAL(activated()), mMapDocument, SLOT(setIntervalOffset7()));
        disconnect(mIntervalOffsetShortcut8, SIGNAL(activated()), mMapDocument, SLOT(setIntervalOffset8()));
        disconnect(mChangeLayerShortcut, SIGNAL(activated()), mMapDocument, SLOT(selectNextLayer()));
        disconnect(mChangeLayerBackShortcut, SIGNAL(activated()), mMapDocument, SLOT(selectPreviousLayer()));

        disconnect(mMapDocument->validatorModel(), SIGNAL(highlightToolbarAction(int))
                                       , mToolManager, SLOT(highlightToolbarAction(int)));
    }

    if (mZoomable) {
        mZoomable->connectToComboBox(0);

        disconnect(mZoomable, SIGNAL(scaleChanged(qreal)),
                   this, SLOT(updateZoomLabel()));
    }
    mZoomable = 0;

    mMapDocument = mapDocument;

    mActionHandler->setMapDocument(mapDocument);
    mLayerDock->setMapDocument(mapDocument);
    mMiniMapDock->setMapDocument(mapDocument);
    mToolManager->setMapDocument(mapDocument);
    mTileSelectionManager->setMapDocument(mapDocument);
    mValidator->setMapDocument(mapDocument);
    mValidatorDock->setMapDocument(mapDocument);

    // enable only if the exe can be found and mapDocument exists
    if(!Preferences::instance()->gameDirectory().isEmpty())
        mPlayLevelAction->setEnabled(mapDocument);

    if (mapDocument) {
        connect(mapDocument, SIGNAL(fileNameChanged(QString,QString)),
                SLOT(updateWindowTitle()));
        connect(mapDocument, SIGNAL(currentLayerIndexChanged(int)),
                SLOT(updateActions()));
        connect(mapDocument, SIGNAL(selectedAreaChanged(QRegion,QRegion)),
                SLOT(updateActions()));
        connect(mapDocument, SIGNAL(selectedObjectsChanged()),
                SLOT(updateActions()));

        if (MapView *mapView = mDocumentManager->currentMapView()) {
            mZoomable = mapView->zoomable();
            mZoomable->connectToComboBox(mZoomComboBox);

            connect(mZoomable, SIGNAL(scaleChanged(qreal)),
                    this, SLOT(updateZoomLabel()));

            // RTB: connect for selection click
            connect(mapView->mapScene(), SIGNAL(selectionClick()),
                        this, SLOT(activateObjectSelectionTool()));

            connect(mapDocument, SIGNAL(currentLayerIndexChanged(int)),
                        mapView, SLOT(updateLayerLabelText(int)));
        }

        // RTB: connect Shortcuts
        connect(mFloorLayerShortcut, SIGNAL(activated()), mapDocument, SLOT(selectFloorLayer()));
        connect(mOrbLayerShortcut, SIGNAL(activated()), mapDocument, SLOT(selectOrbLayer()));
        connect(mObjectLayerShortcut, SIGNAL(activated()), mapDocument, SLOT(selectObjectLayer()));
        connect(mIntervalSpeedShortcut1, SIGNAL(activated()), mapDocument, SLOT(setIntervalSpeed1()));
        connect(mIntervalSpeedShortcut2, SIGNAL(activated()), mapDocument, SLOT(setIntervalSpeed2()));
        connect(mIntervalSpeedShortcut3, SIGNAL(activated()), mapDocument, SLOT(setIntervalSpeed3()));
        connect(mIntervalSpeedShortcut4, SIGNAL(activated()), mapDocument, SLOT(setIntervalSpeed4()));
        connect(mIntervalOffsetShortcut1, SIGNAL(activated()), mapDocument, SLOT(setIntervalOffset1()));
        connect(mIntervalOffsetShortcut2, SIGNAL(activated()), mapDocument, SLOT(setIntervalOffset2()));
        connect(mIntervalOffsetShortcut3, SIGNAL(activated()), mapDocument, SLOT(setIntervalOffset3()));
        connect(mIntervalOffsetShortcut4, SIGNAL(activated()), mapDocument, SLOT(setIntervalOffset4()));
        connect(mIntervalOffsetShortcut5, SIGNAL(activated()), mapDocument, SLOT(setIntervalOffset5()));
        connect(mIntervalOffsetShortcut6, SIGNAL(activated()), mapDocument, SLOT(setIntervalOffset6()));
        connect(mIntervalOffsetShortcut7, SIGNAL(activated()), mapDocument, SLOT(setIntervalOffset7()));
        connect(mIntervalOffsetShortcut8, SIGNAL(activated()), mapDocument, SLOT(setIntervalOffset8()));
        connect(mChangeLayerShortcut, SIGNAL(activated()), mapDocument, SLOT(selectNextLayer()));
        connect(mChangeLayerBackShortcut, SIGNAL(activated()), mapDocument, SLOT(selectPreviousLayer()));

        connect(mapDocument, SIGNAL(highlightToolbarAction(int))
                                       , mToolManager, SLOT(highlightToolbarAction(int)));
    }

    updateWindowTitle();
    updateActions();
}

void MainWindow::closeMapDocument(int index)
{
    if (confirmSave(mDocumentManager->documents().at(index)))
        mDocumentManager->closeDocumentAt(index);
}

void MainWindow::reloadError(const QString &error)
{
    QMessageBox::critical(this, tr("Error Reloading Map"), error);
}

void MainWindow::activateObjectSelectionTool()
{
    if(mToolManager->selectedTool() != mObjectSelectionTool)
    {
        mToolManager->selectTool(mObjectSelectionTool);
    }
}

void MainWindow::activateObjectSelectionTool(MapObject *mapObject)
{
    if(mapObject)
    {
        int id = mapObject->cell().tile->id();
        if(id > RTBMapSettings::OrbBorder && mMapDocument->currentLayerIndex() != RTBMapSettings::ObjectID)
            mMapDocument->setCurrentLayerIndex(RTBMapSettings::ObjectID);
        else if(id > RTBMapSettings::FloorBorder && id <= RTBMapSettings::OrbBorder
                && mMapDocument->currentLayerIndex() != RTBMapSettings::OrbObjectID)
            mMapDocument->setCurrentLayerIndex(RTBMapSettings::OrbObjectID);
    }

    if(mToolManager->selectedTool() != mObjectSelectionTool)
        mToolManager->selectTool(mObjectSelectionTool);
}

void MainWindow::setShowPropVisualization(bool show)
{
    if(!mShowPropVisualization && mShowPropVisualization == show)
        return;

    mShowPropVisualization = show;
    Preferences::instance()->setShowPropertyVisualization(show);

    if(mDocumentManager && mDocumentManager->currentMapView())
        mDocumentManager->currentMapView()->mapScene()->update();
}

bool MainWindow::saveFileAsJSON()
{
    if (!mMapDocument)
        return false;

    PluginManager *pm = PluginManager::instance();
    QList<MapWriterInterface*> writers = pm->interfaces<MapWriterInterface>();
    QString filter;
    foreach (const MapWriterInterface *writer, writers) {
        foreach (const QString &str, writer->nameFilters()) {
            if (!str.isEmpty() && str.contains(tr(".json"))) {
                filter += str;
                break;
            }
        }
    }

    QString suggestedFileName;
    if (mMapDocument && !mMapDocument->fileName().isEmpty()) {
        suggestedFileName = mMapDocument->fileName();
    } else {
        QString mapName = mMapDocument->map()->rtbMap()->levelName();
        if(mapName.isEmpty()){
            mapName = tr("untitled");
        } else {
            mapName.replace(QLatin1String(" "), QLatin1String("_"));
        }
        suggestedFileName = fileDialogStartLocation();
        suggestedFileName += QLatin1Char('/');
        suggestedFileName += mapName + QLatin1String(".json");
    }

    QString selectedFilter = filter;
    const QString fileName =
            QFileDialog::getSaveFileName(this, QString(), suggestedFileName,
                                         filter, &selectedFilter);

    if (fileName.isEmpty())
        return false;

    QString writerPluginFilename;
    if (const Plugin *p = pm->pluginByNameFilter(selectedFilter))
        writerPluginFilename = p->fileName;

    mMapDocument->setWriterPluginFileName(writerPluginFilename);

    return saveFile(fileName);
}

void MainWindow::buildMap()
{
    // save first
    if(!saveFile())
        return;

    RTBCore *core = RTBCore::instance();

    if(mValidator->hasError())
    {
        QMessageBox *messageBox = new QMessageBox(this);
        messageBox->setText(QLatin1String("Please fix all errors in the level (as listed in the Validator view) before playing it."));
        messageBox->setVisible(true);
    }
    // check if game is already running
    else if(!core->isGameAlreadyRunning())
    {
        core->buildMap(mMapDocument);
    }
    else
    {
        QMessageBox *messageBox = new QMessageBox(this);
        messageBox->setText(QLatin1String("Road to Ballhalla is already running. To reload the level and see your changes, press F5 in the game."));
        messageBox->setVisible(true);
    }
}

void MainWindow::highlightSection(int section)
{
    mHighlightSection = section;
    // to update the section highlight
    repaint();
}

void MainWindow::paintEvent(QPaintEvent * event)
{
    QMainWindow::paintEvent(event);

    if(mHighlightSection == RTBTutorial::None)
        return;


    QPainter painter(this);
    painter.setPen(QPen(Qt::red, 2));
    QRectF sectionRect;

    switch (mHighlightSection) {
    case RTBTutorial::MainToolBar:
        sectionRect = mUi->mainToolBar->geometry();
        break;
    case RTBTutorial::ToolsToolBar:
        sectionRect = mUi->toolsToolBar->geometry();
        break;
    case RTBTutorial::LayerDock:
        sectionRect = mLayerDock->geometry();
        break;
    case RTBTutorial::PropertiesDock:
        sectionRect = mPropertiesDock->geometry();
        break;
    case RTBTutorial::ValidatorDock:
        sectionRect = mValidatorDock->geometry();
        break;
    case RTBTutorial::MapView:
        sectionRect = mDocumentManager->widget()->geometry();
        break;
    default:
        break;
    }

    if(mHighlightSection == RTBTutorial::MapView)
    {
        sectionRect.setTop(sectionRect.top() - 2);
        sectionRect.setBottom(sectionRect.bottom() + 2);
        sectionRect.setLeft(sectionRect.left() - 2);
        sectionRect.setRight(sectionRect.right() + 2);
    }
    else if(mHighlightSection == RTBTutorial::PropertiesDock
            || mHighlightSection == RTBTutorial::LayerDock
            || mHighlightSection == RTBTutorial::ValidatorDock)
    {
        sectionRect.setTop(sectionRect.top() - 2);
        sectionRect.setBottom(sectionRect.bottom() - 2);
        sectionRect.setLeft(sectionRect.left() - 0);
        sectionRect.setRight(sectionRect.right() + 0);
    }
    else
    {
        sectionRect.setTop(sectionRect.top() + 2);
        sectionRect.setBottom(sectionRect.bottom() - 2);
        sectionRect.setLeft(sectionRect.left() + 2);
        sectionRect.setRight(sectionRect.right() - 2);
    }

    painter.drawRect(sectionRect);
}
