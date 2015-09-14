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
#include "mapformat.h"
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
#include "tile.h"
#include "tilelayer.h"
#include "tileselectiontool.h"
#include "tileset.h"
#include "tilesetdock.h"
#include "tilesetmanager.h"
#include "tilestampmanager.h"
#include "tilestampsdock.h"
#include "terraindock.h"
#include "toolmanager.h"
#include "undodock.h"
#include "utils.h"
#include "zoomable.h"
#include "commandbutton.h"
#include "objectsdock.h"
#include "minimapdock.h"
#include "consoledock.h"
#include "tileanimationeditor.h"
#include "tilecollisioneditor.h"
#include "tmxmapformat.h"
#include "imagemovementtool.h"
#include "magicwandtool.h"
#include "selectsametiletool.h"

#ifdef Q_OS_MAC
#include "macsupport.h"
#endif

#include <QMimeData>
#include <QCloseEvent>
#include <QComboBox>
#include <QDesktopServices>
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
    , mObjectsDock(new ObjectsDock())
    , mTilesetDock(new TilesetDock(this))
    , mTerrainDock(new TerrainDock(this))
    , mMiniMapDock(new MiniMapDock(this))
    , mConsoleDock(new ConsoleDock(this))
    , mTileAnimationEditor(new TileAnimationEditor(this))
    , mTileCollisionEditor(new TileCollisionEditor(this))
    , mCurrentLayerLabel(new QLabel)
    , mZoomable(0)
    , mZoomComboBox(new QComboBox)
    , mStatusInfoLabel(new QLabel)
    , mAutomappingManager(new AutomappingManager(this))
    , mDocumentManager(DocumentManager::instance())
    , mToolManager(new ToolManager(this))
    , mTileStampManager(new TileStampManager(*mToolManager, this))
{
    mUi->setupUi(this);
    setCentralWidget(mDocumentManager->widget());

#ifdef Q_OS_MAC
    MacSupport::addFullscreen(this);
#endif

    Preferences *preferences = Preferences::instance();

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
#if QT_VERSION >= 0x050500
    undoAction->setPriority(QAction::LowPriority);
#endif
    redoAction->setPriority(QAction::LowPriority);
    redoAction->setIcon(redoIcon);
    undoAction->setIcon(undoIcon);
    redoAction->setIconText(tr("Redo"));
    undoAction->setIconText(tr("Undo"));
    connect(undoGroup, SIGNAL(cleanChanged(bool)), SLOT(updateWindowTitle()));

    UndoDock *undoDock = new UndoDock(undoGroup, this);
    PropertiesDock *propertiesDock = new PropertiesDock(this);
    TileStampsDock *tileStampsDock = new TileStampsDock(mTileStampManager, this);

    addDockWidget(Qt::RightDockWidgetArea, mLayerDock);
    addDockWidget(Qt::LeftDockWidgetArea, propertiesDock);
    addDockWidget(Qt::LeftDockWidgetArea, undoDock);
    addDockWidget(Qt::LeftDockWidgetArea, mMapsDock);
    addDockWidget(Qt::RightDockWidgetArea, mObjectsDock);
    addDockWidget(Qt::RightDockWidgetArea, mMiniMapDock);
    addDockWidget(Qt::RightDockWidgetArea, mTerrainDock);
    addDockWidget(Qt::RightDockWidgetArea, mTilesetDock);
    addDockWidget(Qt::BottomDockWidgetArea, mConsoleDock);
    addDockWidget(Qt::LeftDockWidgetArea, tileStampsDock);

    tabifyDockWidget(mMiniMapDock, mObjectsDock);
    tabifyDockWidget(mObjectsDock, mLayerDock);
    tabifyDockWidget(mTerrainDock, mTilesetDock);
    tabifyDockWidget(undoDock, mMapsDock);
    tabifyDockWidget(tileStampsDock, undoDock);

    // These dock widgets may not be immediately useful to many people, so
    // they are hidden by default.
    undoDock->setVisible(false);
    mMapsDock->setVisible(false);
    mConsoleDock->setVisible(false);
    tileStampsDock->setVisible(false);

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
    QList<QKeySequence> deleteKeys = QKeySequence::keyBindings(QKeySequence::Delete);
#ifdef Q_OS_OSX
    // Add the Backspace key as primary shortcut for Delete, which seems to be
    // the expected one for OS X.
    if (!deleteKeys.contains(QKeySequence(Qt::Key_Backspace)))
        deleteKeys.prepend(QKeySequence(Qt::Key_Backspace));
#endif
    mUi->actionDelete->setShortcuts(deleteKeys);

    undoAction->setShortcuts(QKeySequence::Undo);
    redoAction->setShortcuts(QKeySequence::Redo);

    mUi->actionShowGrid->setChecked(preferences->showGrid());
    mUi->actionShowTileObjectOutlines->setChecked(preferences->showTileObjectOutlines());
    mUi->actionShowTileAnimations->setChecked(preferences->showTileAnimations());
    mUi->actionSnapToGrid->setChecked(preferences->snapToGrid());
    mUi->actionSnapToFineGrid->setChecked(preferences->snapToFineGrid());
    mUi->actionHighlightCurrentLayer->setChecked(preferences->highlightCurrentLayer());

    QActionGroup *objectLabelVisibilityGroup = new QActionGroup(this);
    mUi->actionNoLabels->setActionGroup(objectLabelVisibilityGroup);
    mUi->actionLabelsForSelectedObjects->setActionGroup(objectLabelVisibilityGroup);
    mUi->actionLabelsForAllObjects->setActionGroup(objectLabelVisibilityGroup);

    switch (preferences->objectLabelVisibility()) {
    case Preferences::NoObjectLabels:
        mUi->actionNoLabels->setChecked(true);
        break;
    case Preferences::SelectedObjectLabels:
        mUi->actionLabelsForSelectedObjects->setChecked(true);
        break;
    case Preferences::AllObjectLabels:
        mUi->actionLabelsForAllObjects->setChecked(true);
        break;
    }

    connect(objectLabelVisibilityGroup, &QActionGroup::triggered,
            this, &MainWindow::labelVisibilityActionTriggered);

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

    mCommandButton = new CommandButton(this);
    mUi->mainToolBar->addWidget(mCommandButton);

    mUi->menuMap->insertAction(mUi->actionOffsetMap,
                               mActionHandler->actionCropToSelection());

    mRandomButton = new QToolButton(this);
    mRandomButton->setToolTip(tr("Random Mode"));
    mRandomButton->setIcon(QIcon(QLatin1String(":images/24x24/dice.png")));
    mRandomButton->setCheckable(true);
    mRandomButton->setShortcut(QKeySequence(tr("D")));
    mUi->mainToolBar->addWidget(mRandomButton);

    mLayerMenu = new QMenu(tr("&Layer"), this);
    mLayerMenu->addAction(mActionHandler->actionAddTileLayer());
    mLayerMenu->addAction(mActionHandler->actionAddObjectGroup());
    mLayerMenu->addAction(mActionHandler->actionAddImageLayer());
    mLayerMenu->addAction(mActionHandler->actionDuplicateLayer());
    mLayerMenu->addAction(mActionHandler->actionMergeLayerDown());
    mLayerMenu->addAction(mActionHandler->actionRemoveLayer());
    mLayerMenu->addSeparator();
    mLayerMenu->addAction(mActionHandler->actionSelectPreviousLayer());
    mLayerMenu->addAction(mActionHandler->actionSelectNextLayer());
    mLayerMenu->addAction(mActionHandler->actionMoveLayerUp());
    mLayerMenu->addAction(mActionHandler->actionMoveLayerDown());
    mLayerMenu->addSeparator();
    mLayerMenu->addAction(mActionHandler->actionToggleOtherLayers());
    mLayerMenu->addSeparator();
    mLayerMenu->addAction(mActionHandler->actionLayerProperties());

    menuBar()->insertMenu(mUi->menuHelp->menuAction(), mLayerMenu);

    connect(mUi->actionNew, SIGNAL(triggered()), SLOT(newMap()));
    connect(mUi->actionOpen, SIGNAL(triggered()), SLOT(openFile()));
    connect(mUi->actionClearRecentFiles, SIGNAL(triggered()),
            SLOT(clearRecentFiles()));
    connect(mUi->actionSave, SIGNAL(triggered()), SLOT(saveFile()));
    connect(mUi->actionSaveAs, SIGNAL(triggered()), SLOT(saveFileAs()));
    connect(mUi->actionSaveAll, SIGNAL(triggered()), SLOT(saveAll()));
    connect(mUi->actionExportAsImage, SIGNAL(triggered()), SLOT(exportAsImage()));
    connect(mUi->actionExport, SIGNAL(triggered()), SLOT(export_()));
    connect(mUi->actionExportAs, SIGNAL(triggered()), SLOT(exportAs()));
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
    connect(mUi->actionOffsetMap, SIGNAL(triggered()), SLOT(offsetMap()));
    connect(mUi->actionMapProperties, SIGNAL(triggered()),
            SLOT(editMapProperties()));
    connect(mUi->actionAutoMap, SIGNAL(triggered()),
            mAutomappingManager, SLOT(autoMap()));

    connect(mUi->actionDocumentation, SIGNAL(triggered()), SLOT(openDocumentation()));
    connect(mUi->actionBecomePatron, SIGNAL(triggered()), SLOT(becomePatron()));
    connect(mUi->actionAbout, SIGNAL(triggered()), SLOT(aboutTiled()));

    connect(mTilesetDock, SIGNAL(tilesetsDropped(QStringList)),
            SLOT(newTilesets(QStringList)));

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
    setThemeIcon(mUi->actionDocumentation, "help-contents");
    setThemeIcon(mUi->actionAbout, "help-about");

    mStampBrush = new StampBrush(this);
    mTerrainBrush = new TerrainBrush(this);
    mBucketFillTool = new BucketFillTool(this);
    CreateObjectTool *tileObjectsTool = new CreateTileObjectTool(this);
    CreateObjectTool *rectangleObjectsTool = new CreateRectangleObjectTool(this);
    CreateObjectTool *ellipseObjectsTool = new CreateEllipseObjectTool(this);
    CreateObjectTool *polygonObjectsTool = new CreatePolygonObjectTool(this);
    CreateObjectTool *polylineObjectsTool = new CreatePolylineObjectTool(this);

    connect(mTilesetDock, SIGNAL(stampCaptured(TileStamp)),
            this, SLOT(setStamp(TileStamp)));
    connect(mStampBrush, SIGNAL(stampCaptured(TileStamp)),
            this, SLOT(setStamp(TileStamp)));

    connect(mTilesetDock, &TilesetDock::currentTileChanged,
            tileObjectsTool, &CreateObjectTool::setTile);
    connect(mTilesetDock, &TilesetDock::currentTileChanged,
            mTileAnimationEditor, &TileAnimationEditor::setTile);
    connect(mTilesetDock, &TilesetDock::currentTileChanged,
            mTileCollisionEditor, &TileCollisionEditor::setTile);
    connect(mTilesetDock, SIGNAL(newTileset()),
            this, SLOT(newTileset()));

    connect(mTerrainDock, SIGNAL(currentTerrainChanged(const Terrain*)),
            this, SLOT(setTerrainBrush(const Terrain*)));

    connect(tileStampsDock, SIGNAL(setStamp(TileStamp)),
            this, SLOT(setStamp(TileStamp)));

    connect(mRandomButton, SIGNAL(toggled(bool)),
            mStampBrush, SLOT(setRandom(bool)));
    connect(mRandomButton, SIGNAL(toggled(bool)),
            mBucketFillTool, SLOT(setRandom(bool)));

    QToolBar *toolBar = mUi->toolsToolBar;
    toolBar->addAction(mToolManager->registerTool(mStampBrush));
    toolBar->addAction(mToolManager->registerTool(mTerrainBrush));
    toolBar->addAction(mToolManager->registerTool(mBucketFillTool));
    toolBar->addAction(mToolManager->registerTool(new Eraser(this)));
    toolBar->addAction(mToolManager->registerTool(new TileSelectionTool(this)));
    toolBar->addAction(mToolManager->registerTool(new MagicWandTool(this)));
    toolBar->addAction(mToolManager->registerTool(new SelectSameTileTool(this)));
    toolBar->addSeparator();
    toolBar->addAction(mToolManager->registerTool(new ObjectSelectionTool(this)));
    toolBar->addAction(mToolManager->registerTool(new EditPolygonTool(this)));
    toolBar->addAction(mToolManager->registerTool(rectangleObjectsTool));
    toolBar->addAction(mToolManager->registerTool(ellipseObjectsTool));
    toolBar->addAction(mToolManager->registerTool(polygonObjectsTool));
    toolBar->addAction(mToolManager->registerTool(polylineObjectsTool));
    toolBar->addAction(mToolManager->registerTool(tileObjectsTool));
    toolBar->addSeparator();
    toolBar->addAction(mToolManager->registerTool(new ImageMovementTool(this)));

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
    mShowTileAnimationEditor = new QAction(tr("Tile Animation Editor"), this);
    mShowTileAnimationEditor->setCheckable(true);
    mShowTileCollisionEditor = new QAction(tr("Tile Collision Editor"), this);
    mShowTileCollisionEditor->setCheckable(true);
    mShowTileCollisionEditor->setShortcut(tr("Ctrl+Shift+O"));
    mShowTileCollisionEditor->setShortcutContext(Qt::ApplicationShortcut);
    QMenu *popupMenu = createPopupMenu();
    popupMenu->setParent(this);
    mViewsAndToolbarsMenu->setMenu(popupMenu);
    mUi->menuView->insertAction(mUi->actionShowGrid, mViewsAndToolbarsMenu);
    mUi->menuView->insertAction(mUi->actionShowGrid, mShowTileAnimationEditor);
    mUi->menuView->insertAction(mUi->actionShowGrid, mShowTileCollisionEditor);
    mUi->menuView->insertSeparator(mUi->actionShowGrid);

    connect(mShowTileAnimationEditor, SIGNAL(toggled(bool)),
            mTileAnimationEditor, SLOT(setVisible(bool)));
    connect(mTileAnimationEditor, SIGNAL(closed()), SLOT(onAnimationEditorClosed()));

    connect(mShowTileCollisionEditor, SIGNAL(toggled(bool)),
            mTileCollisionEditor, SLOT(setVisible(bool)));
    connect(mTileCollisionEditor, SIGNAL(closed()), SLOT(onCollisionEditorClosed()));

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


    new QShortcut(tr("X"), this, SLOT(flipHorizontally()));
    new QShortcut(tr("Y"), this, SLOT(flipVertically()));
    new QShortcut(tr("Z"), this, SLOT(rotateRight()));
    new QShortcut(tr("Shift+Z"), this, SLOT(rotateLeft()));

    QShortcut *copyPositionShortcut = new QShortcut(tr("Alt+C"), this);
    connect(copyPositionShortcut, SIGNAL(activated()),
            mActionHandler, SLOT(copyPosition()));

    updateActions();
    readSettings();
    setupQuickStamps();

    connect(mAutomappingManager, SIGNAL(warningsOccurred(bool)),
            this, SLOT(autoMappingWarning(bool)));
    connect(mAutomappingManager, SIGNAL(errorsOccurred(bool)),
            this, SLOT(autoMappingError(bool)));
}

MainWindow::~MainWindow()
{
    mDocumentManager->closeAllDocuments();

    // This needs to happen before deleting the TilesetManager otherwise it may
    // hold references to tilesets.
    mTileAnimationEditor->setTile(nullptr);
    mTileAnimationEditor->writeSettings();
    mTileCollisionEditor->setTile(nullptr);
    mTileCollisionEditor->writeSettings();

    delete mTileStampManager;
    mTileStampManager = nullptr;

    delete mStampBrush;
    mStampBrush = nullptr;

    delete mBucketFillTool;
    mBucketFillTool = nullptr;

    TilesetManager::deleteInstance();
    DocumentManager::deleteInstance();
    Preferences::deleteInstance();
    LanguageManager::deleteInstance();
    PluginManager::deleteInstance();
    ClipboardManager::deleteInstance();

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
                          MapFormat *format)
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
    MapDocument *mapDocument = MapDocument::load(fileName, format, &error);
    if (!mapDocument) {
        QMessageBox::critical(this, tr("Error Opening Map"), error);
        return false;
    }

    mDocumentManager->addDocument(mapDocument);
    setRecentFile(fileName);
    return true;
}

bool MainWindow::openFile(const QString &fileName)
{
    return openFile(fileName, 0);
}

void MainWindow::openLastFiles()
{
    mSettings.beginGroup(QLatin1String("recentFiles"));

    QStringList lastOpenFiles = mSettings.value(
                QLatin1String("lastOpenFiles")).toStringList();
    QVariant openCountVariant = mSettings.value(
                QLatin1String("recentOpenedFiles"));

    // Backwards compatibility mode
    if (openCountVariant.isValid()) {
        const QStringList recentFiles = mSettings.value(
                    QLatin1String("fileNames")).toStringList();
        int openCount = qMin(openCountVariant.toInt(), recentFiles.size());
        for (; openCount; --openCount)
            lastOpenFiles.append(recentFiles.at(openCount - 1));
        mSettings.remove(QLatin1String("recentOpenedFiles"));
    }

    QStringList mapScales = mSettings.value(
                QLatin1String("mapScale")).toStringList();
    QStringList scrollX = mSettings.value(
                QLatin1String("scrollX")).toStringList();
    QStringList scrollY = mSettings.value(
                QLatin1String("scrollY")).toStringList();
    QStringList selectedLayer = mSettings.value(
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
            mSettings.value(QLatin1String("lastActive")).toString();
    int documentIndex = mDocumentManager->findDocument(lastActiveDocument);
    if (documentIndex != -1)
        mDocumentManager->switchToDocument(documentIndex);

    mSettings.endGroup();
}

void MainWindow::openFile()
{
    QString filter = tr("All Files (*)");
    filter += QLatin1String(";;");

    QString selectedFilter = tr("Tiled map files (*.tmx)");
    filter += selectedFilter;

    FormatHelper<MapFormat> helper(MapFormat::Read, filter);

    selectedFilter = mSettings.value(QLatin1String("lastUsedOpenFilter"),
                                     selectedFilter).toString();

    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open Map"),
                                                          fileDialogStartLocation(),
                                                          helper.filter(),
                                                          &selectedFilter);
    if (fileNames.isEmpty())
        return;

    // When a particular filter was selected, use the associated format
    MapFormat *mapFormat = helper.formatByNameFilter(selectedFilter);

    mSettings.setValue(QLatin1String("lastUsedOpenFilter"), selectedFilter);
    foreach (const QString &fileName, fileNames)
        openFile(fileName, mapFormat);
}

bool MainWindow::saveFile(const QString &fileName)
{
    if (!mMapDocument)
        return false;

    if (fileName.isEmpty())
        return false;

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

    if (currentFileName.isEmpty())
        return saveFileAs();
    else
        return saveFile(currentFileName);
}

bool MainWindow::saveFileAs()
{
    const QString tmxFilter = tr("Tiled map files (*.tmx)");

    FormatHelper<MapFormat> helper(MapFormat::ReadWrite, tmxFilter);

    QString selectedFilter;
    if (mMapDocument) {
        if (MapFormat *format = mMapDocument->writerFormat())
            selectedFilter = format->nameFilter();
    }

    if (selectedFilter.isEmpty())
        selectedFilter = tmxFilter;

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
                                         helper.filter(), &selectedFilter);

    if (fileName.isEmpty())
        return false;

    MapFormat *format = helper.formatByNameFilter(selectedFilter);
    mMapDocument->setWriterFormat(format);

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

    if (!exportFileName.isEmpty()) {
        MapFormat *exportFormat = mMapDocument->exportFormat();
        TmxMapFormat tmxFormat;

        if (!exportFormat)
            exportFormat = &tmxFormat;

        if (exportFormat->write(mMapDocument->map(), exportFileName)) {
            statusBar()->showMessage(tr("Exported to %1").arg(exportFileName),
                                     3000);
            return;
        }

        QMessageBox::critical(this, tr("Error Exporting Map"),
                              exportFormat->errorString());
    }

    // fall back when no successful export happened
    exportAs();
}

void MainWindow::exportAs()
{
    if (!mMapDocument)
        return;

    FormatHelper<MapFormat> helper(FileFormat::Write, tr("All Files (*)"));

    Preferences *pref = Preferences::instance();

    QString selectedFilter =
            mSettings.value(QLatin1String("lastUsedExportFilter")).toString();
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
                                                    helper.filter(),
                                                    &selectedFilter,
                                                    QFileDialog::DontConfirmOverwrite);
    if (fileName.isEmpty())
        return;

    // If a specific filter was selected, use that format
    MapFormat *chosenFormat = helper.formatByNameFilter(selectedFilter);

    // If not, try to find the file extension among the name filters
    QString suffix = QFileInfo(fileName).completeSuffix();
    if (!chosenFormat && !suffix.isEmpty()) {
        suffix.prepend(QLatin1String("*."));

        for (MapFormat *format : helper.formats()) {
            if (format->nameFilter().contains(suffix, Qt::CaseInsensitive)) {
                if (chosenFormat) {
                    QMessageBox::warning(this, tr("Non-unique file extension"),
                                         tr("Non-unique file extension.\n"
                                            "Please select specific format."));
                    return exportAs();
                } else {
                    chosenFormat = format;
                }
            }
        }
    }

    // Also support exporting to the TMX map format when requested
    TmxMapFormat tmxMapFormat;
    if (!chosenFormat && tmxMapFormat.supportsFile(fileName))
        chosenFormat = &tmxMapFormat;

    if (!chosenFormat) {
        QMessageBox::critical(this, tr("Unknown File Format"),
                              tr("The given filename does not have any known "
                                 "file extension."));
        return;
    }

    // Check if writer will overwrite existing files here because some writers
    // could save to multiple files at the same time. For example CSV saves
    // each layer into a separate file.
    QStringList outputFiles = chosenFormat->outputFiles(mMapDocument->map(),
                                                        fileName);
    if (outputFiles.size() > 0) {
        // Check if any output file already exists
        QString message =
                tr("Some export files already exist:") + QLatin1String("\n\n");

        bool overwriteHappens = false;

        for (const QString &outputFile : outputFiles) {
            if (QFile::exists(outputFile)) {
                overwriteHappens = true;
                message += outputFile + QLatin1Char('\n');
            }
        }
        message += QLatin1Char('\n') + tr("Do you want to replace them?");

        // If overwrite happens, warn the user and get confirmation before exporting
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
    mSettings.setValue(QLatin1String("lastUsedExportFilter"), selectedFilter);

    if (!chosenFormat->write(mMapDocument->map(), fileName)) {
        QMessageBox::critical(this, tr("Error Exporting Map"),
                              chosenFormat->errorString());
    } else {
        // Remember export parameters, so subsequent exports can be done faster
        mMapDocument->setLastExportFileName(fileName);
        if (chosenFormat != &tmxMapFormat)
            mMapDocument->setExportFormat(chosenFormat);
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

    copy();

    QUndoStack *stack = mMapDocument->undoStack();
    stack->beginMacro(tr("Cut"));

    if (tileLayer && !selectedArea.isEmpty()) {
        stack->push(new EraseTiles(mMapDocument, tileLayer, selectedArea));
    } else if (!selectedObjects.isEmpty()) {
        foreach (MapObject *mapObject, selectedObjects)
            stack->push(new RemoveMapObject(mMapDocument, mapObject));
    }

    mActionHandler->selectNone();

    stack->endMacro();
}

void MainWindow::copy()
{
    if (!mMapDocument)
        return;

    ClipboardManager::instance()->copySelection(mMapDocument);
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

    // We can currently only handle maps with a single layer
    if (map->layerCount() != 1)
        return;

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->addReferences(map->tilesets());

    mMapDocument->unifyTilesets(map.data());
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

void MainWindow::openPreferences()
{
    PreferencesDialog preferencesDialog(this);
    preferencesDialog.exec();
}

void MainWindow::labelVisibilityActionTriggered(QAction *action)
{
    Preferences::ObjectLabelVisiblity visibility = Preferences::NoObjectLabels;

    if (action == mUi->actionLabelsForSelectedObjects)
        visibility = Preferences::SelectedObjectLabels;
    else if (action == mUi->actionLabelsForAllObjects)
        visibility = Preferences::AllObjectLabels;

    Preferences::instance()->setObjectLabelVisibility(visibility);
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
    if (!mMapDocument)
        return;

    Map *map = mMapDocument->map();
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

    for (const QString &fileName : fileNames) {
        QString error;
        SharedTileset tileset = Tiled::readTileset(fileName, &error);
        if (tileset) {
            tilesets.append(tileset);
        } else if (fileNames.size() == 1) {
            QMessageBox::critical(this, tr("Error Reading Tileset"), error);
            return;
        } else {
            int result;

            result = QMessageBox::warning(this, tr("Error Reading Tileset"),
                                          tr("%1: %2").arg(fileName, error),
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

void MainWindow::autoMappingError(bool automatic)
{
    const QString title = tr("Automatic Mapping Error");
    QString error = mAutomappingManager->errorString();
    if (!error.isEmpty()) {
        if (automatic)
            statusBar()->showMessage(error, 3000);
        else
            QMessageBox::critical(this, title, error);
    }
}

void MainWindow::autoMappingWarning(bool automatic)
{
    const QString title = tr("Automatic Mapping Warning");
    QString warning = mAutomappingManager->warningString();
    if (!warning.isEmpty()) {
        if (automatic)
            statusBar()->showMessage(warning, 3000);
        else
            QMessageBox::warning(this, title, warning);
    }
}

void MainWindow::onAnimationEditorClosed()
{
    mShowTileAnimationEditor->setChecked(false);
}

void MainWindow::onCollisionEditorClosed()
{
    mShowTileCollisionEditor->setChecked(false);
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        openFile(action->data().toString());
}

QStringList MainWindow::recentFiles() const
{
    QVariant v = mSettings.value(QLatin1String("recentFiles/fileNames"));
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

    mSettings.beginGroup(QLatin1String("recentFiles"));
    mSettings.setValue(QLatin1String("fileNames"), files);
    mSettings.endGroup();
    updateRecentFiles();
}

void MainWindow::clearRecentFiles()
{
    mSettings.beginGroup(QLatin1String("recentFiles"));
    mSettings.setValue(QLatin1String("fileNames"), QStringList());
    mSettings.endGroup();
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
    mUi->actionExport->setEnabled(map);
    mUi->actionExportAs->setEnabled(map);
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
    mUi->actionOffsetMap->setEnabled(map);
    mUi->actionMapProperties->setEnabled(map);
    mUi->actionAutoMap->setEnabled(map);

    mCommandButton->setEnabled(map);

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

void MainWindow::openDocumentation()
{
    QDesktopServices::openUrl(QUrl(QLatin1String("http://doc.mapeditor.org")));
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

    mTilesetDock->selectTilesInStamp(stamp);
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
    mSettings.beginGroup(QLatin1String("mainwindow"));
    mSettings.setValue(QLatin1String("geometry"), saveGeometry());
    mSettings.setValue(QLatin1String("state"), saveState());
    mSettings.endGroup();

    mSettings.beginGroup(QLatin1String("recentFiles"));
    if (MapDocument *document = mDocumentManager->currentDocument())
        mSettings.setValue(QLatin1String("lastActive"), document->fileName());

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
    mSettings.setValue(QLatin1String("lastOpenFiles"), fileList);
    mSettings.setValue(QLatin1String("mapScale"), mapScales);
    mSettings.setValue(QLatin1String("scrollX"), scrollX);
    mSettings.setValue(QLatin1String("scrollY"), scrollY);
    mSettings.setValue(QLatin1String("selectedLayer"), selectedLayer);
    mSettings.endGroup();
}

void MainWindow::readSettings()
{
    mSettings.beginGroup(QLatin1String("mainwindow"));
    QByteArray geom = mSettings.value(QLatin1String("geometry")).toByteArray();
    if (!geom.isEmpty())
        restoreGeometry(geom);
    else
        resize(1200, 700);
    restoreState(mSettings.value(QLatin1String("state"),
                                 QByteArray()).toByteArray());
    mSettings.endGroup();
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

    mRandomButton->setToolTip(tr("Random Mode"));
    mLayerMenu->setTitle(tr("&Layer"));
    mViewsAndToolbarsMenu->setText(tr("Views and Toolbars"));
    mShowTileAnimationEditor->setText(tr("Tile Animation Editor"));
    mShowTileCollisionEditor->setText(tr("Tile Collision Editor"));
    mActionHandler->retranslateUi();
    mToolManager->retranslateTools();
}

void MainWindow::mapDocumentChanged(MapDocument *mapDocument)
{
    if (mMapDocument)
        mMapDocument->disconnect(this);

    if (mZoomable) {
        mZoomable->connectToComboBox(0);

        disconnect(mZoomable, SIGNAL(scaleChanged(qreal)),
                   this, SLOT(updateZoomLabel()));
    }
    mZoomable = 0;

    mMapDocument = mapDocument;

    mActionHandler->setMapDocument(mapDocument);
    mLayerDock->setMapDocument(mapDocument);
    mObjectsDock->setMapDocument(mapDocument);
    mTilesetDock->setMapDocument(mapDocument);
    mTerrainDock->setMapDocument(mapDocument);
    mMiniMapDock->setMapDocument(mapDocument);
    mTileAnimationEditor->setMapDocument(mapDocument);
    mTileCollisionEditor->setMapDocument(mapDocument);
    mToolManager->setMapDocument(mapDocument);
    mAutomappingManager->setMapDocument(mapDocument);

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
        }
    }

    updateWindowTitle();
    updateActions();
}

void MainWindow::setupQuickStamps()
{
    QList<Qt::Key> keys = TileStampManager::quickStampKeys();

    QSignalMapper *selectMapper = new QSignalMapper(this);
    QSignalMapper *createMapper = new QSignalMapper(this);
    QSignalMapper *extendMapper = new QSignalMapper(this);

    for (int i = 0; i < keys.length(); i++) {
        Qt::Key key = keys.at(i);

        // Set up shortcut for selecting this quick stamp
        QShortcut *selectStamp = new QShortcut(key, this);
        connect(selectStamp, SIGNAL(activated()), selectMapper, SLOT(map()));
        selectMapper->setMapping(selectStamp, i);

        // Set up shortcut for creating this quick stamp
        QShortcut *createStamp = new QShortcut(Qt::CTRL + key, this);
        connect(createStamp, SIGNAL(activated()), createMapper, SLOT(map()));
        createMapper->setMapping(createStamp, i);

        // Set up shortcut for extending this quick stamp
        QShortcut *extendStamp = new QShortcut(Qt::CTRL + Qt::SHIFT + key, this);
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

void MainWindow::closeMapDocument(int index)
{
    if (confirmSave(mDocumentManager->documents().at(index)))
        mDocumentManager->closeDocumentAt(index);
}

void MainWindow::reloadError(const QString &error)
{
    QMessageBox::critical(this, tr("Error Reloading Map"), error);
}
