/*
 * mainwindow.cpp
 * Copyright 2008-2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#include "documentmanager.h"
#include "editpolygontool.h"
#include "eraser.h"
#include "erasetiles.h"
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
#include "mappropertiesdialog.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "newmapdialog.h"
#include "newtilesetdialog.h"
#include "pluginmanager.h"
#include "propertiesdialog.h"
#include "resizedialog.h"
#include "objectselectiontool.h"
#include "objectgroup.h"
#include "offsetmapdialog.h"
#include "preferences.h"
#include "preferencesdialog.h"
#include "quickstampmanager.h"
#include "saveasimagedialog.h"
#include "stampbrush.h"
#include "terrainbrush.h"
#include "tilelayer.h"
#include "tileselectiontool.h"
#include "tileset.h"
#include "tilesetdock.h"
#include "tilesetmanager.h"
#include "terraindock.h"
#include "toolmanager.h"
#include "tmxmapreader.h"
#include "tmxmapwriter.h"
#include "undodock.h"
#include "utils.h"
#include "zoomable.h"
#include "commandbutton.h"
#include "objectsdock.h"

#ifdef Q_WS_MAC
#include "macsupport.h"
#endif

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

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
    , mUi(new Ui::MainWindow)
    , mMapDocument(0)
    , mActionHandler(new MapDocumentActionHandler(this))
    , mLayerDock(new LayerDock(this))
    , mObjectsDock(new ObjectsDock())
    , mTilesetDock(new TilesetDock(this))
    , mTerrainDock(new TerrainDock(this))
    , mCurrentLayerLabel(new QLabel)
    , mZoomable(0)
    , mZoomComboBox(new QComboBox)
    , mStatusInfoLabel(new QLabel)
    , mClipboardManager(new ClipboardManager(this))
    , mDocumentManager(DocumentManager::instance())
{
    mUi->setupUi(this);
    setCentralWidget(mDocumentManager->widget());

    PluginManager::instance()->loadPlugins();

#ifdef Q_WS_MAC
    MacSupport::addFullscreen(this);
#endif

    Preferences *preferences = Preferences::instance();

    QIcon redoIcon(QLatin1String(":images/16x16/edit-redo.png"));
    QIcon undoIcon(QLatin1String(":images/16x16/edit-undo.png"));

    QIcon tiledIcon(QLatin1String(":images/tiled-icon-16.png"));
    tiledIcon.addFile(QLatin1String(":images/tiled-icon-32.png"));
    setWindowIcon(tiledIcon);

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

    addDockWidget(Qt::RightDockWidgetArea, mLayerDock);
    addDockWidget(Qt::RightDockWidgetArea, undoDock);
    addDockWidget(Qt::RightDockWidgetArea, mObjectsDock);
    addDockWidget(Qt::RightDockWidgetArea, mTerrainDock);
    addDockWidget(Qt::RightDockWidgetArea, mTilesetDock);
    tabifyDockWidget(undoDock, mObjectsDock);
    tabifyDockWidget(mObjectsDock, mLayerDock);
    tabifyDockWidget(mTerrainDock, mTilesetDock);

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
    mUi->actionSnapToGrid->setChecked(preferences->snapToGrid());
    mUi->actionHighlightCurrentLayer->setChecked(preferences->highlightCurrentLayer());

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
    mLayerMenu->addAction(mActionHandler->actionRenameLayer());
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
    connect(mUi->actionSaveAsImage, SIGNAL(triggered()), SLOT(saveAsImage()));
    connect(mUi->actionExport, SIGNAL(triggered()), SLOT(exportAs()));
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
    connect(mUi->actionSnapToGrid, SIGNAL(toggled(bool)),
            preferences, SLOT(setSnapToGrid(bool)));
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
    connect(mUi->actionAutoMap, SIGNAL(triggered()), SLOT(autoMap()));

    connect(mActionHandler->actionLayerProperties(), SIGNAL(triggered()),
            SLOT(editLayerProperties()));

    connect(mUi->actionAbout, SIGNAL(triggered()), SLOT(aboutTiled()));
    connect(mUi->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

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
    setThemeIcon(mUi->actionAbout, "help-about");

    mStampBrush = new StampBrush(this);
    mTerrainBrush = new TerrainBrush(this);
    mBucketFillTool = new BucketFillTool(this);
    CreateObjectTool *tileObjectsTool = new CreateObjectTool(
            CreateObjectTool::CreateTile, this);
    CreateObjectTool *areaObjectsTool = new CreateObjectTool(
            CreateObjectTool::CreateArea, this);
    CreateObjectTool *polygonObjectsTool = new CreateObjectTool(
            CreateObjectTool::CreatePolygon, this);
    CreateObjectTool *polylineObjectsTool = new CreateObjectTool(
            CreateObjectTool::CreatePolyline, this);

    connect(mTilesetDock, SIGNAL(currentTilesChanged(const TileLayer*)),
            this, SLOT(setStampBrush(const TileLayer*)));
    connect(mStampBrush, SIGNAL(currentTilesChanged(const TileLayer*)),
            this, SLOT(setStampBrush(const TileLayer*)));
    connect(mTilesetDock, SIGNAL(currentTileChanged(Tile*)),
            tileObjectsTool, SLOT(setTile(Tile*)));

    connect(mTerrainDock, SIGNAL(currentTerrainChanged(const Terrain*)),
            this, SLOT(setTerrainBrush(const Terrain*)));

    connect(mRandomButton, SIGNAL(toggled(bool)),
            mStampBrush, SLOT(setRandom(bool)));
    connect(mRandomButton, SIGNAL(toggled(bool)),
            mBucketFillTool, SLOT(setRandom(bool)));

    ToolManager *toolManager = ToolManager::instance();
    toolManager->registerTool(mStampBrush);
    toolManager->registerTool(mTerrainBrush);
    toolManager->registerTool(mBucketFillTool);
    toolManager->registerTool(new Eraser(this));
    toolManager->registerTool(new TileSelectionTool(this));
    toolManager->addSeparator();
    toolManager->registerTool(new ObjectSelectionTool(this));
    toolManager->registerTool(new EditPolygonTool(this));
    toolManager->registerTool(areaObjectsTool);
    toolManager->registerTool(tileObjectsTool);
    toolManager->registerTool(polygonObjectsTool);
    toolManager->registerTool(polylineObjectsTool);

    addToolBar(toolManager->toolBar());

    statusBar()->addWidget(mStatusInfoLabel);
    connect(toolManager, SIGNAL(statusInfoChanged(QString)),
            this, SLOT(updateStatusInfoLabel(QString)));
    statusBar()->addWidget(mCurrentLayerLabel);

    mUi->menuView->addSeparator();
    mUi->menuView->addAction(mTilesetDock->toggleViewAction());
    mUi->menuView->addAction(mTerrainDock->toggleViewAction());
    mUi->menuView->addAction(mLayerDock->toggleViewAction());
    mUi->menuView->addAction(undoDock->toggleViewAction());
    mUi->menuView->addAction(mObjectsDock->toggleViewAction());

    connect(mClipboardManager, SIGNAL(hasMapChanged()), SLOT(updateActions()));

    connect(mDocumentManager, SIGNAL(currentDocumentChanged(MapDocument*)),
            SLOT(mapDocumentChanged(MapDocument*)));
    connect(mDocumentManager, SIGNAL(documentCloseRequested(int)),
            this, SLOT(closeMapDocument(int)));

    QShortcut *switchToLeftDocument = new QShortcut(tr("Ctrl+PgUp"), this);
    connect(switchToLeftDocument, SIGNAL(activated()),
            mDocumentManager, SLOT(switchToLeftDocument()));
    QShortcut *switchToLeftDocument1 = new QShortcut(tr("Ctrl+Shift+Tab"), this);
    connect(switchToLeftDocument1, SIGNAL(activated()),
            mDocumentManager, SLOT(switchToLeftDocument()));

    QShortcut *switchToRightDocument = new QShortcut(tr("Ctrl+PgDown"), this);
    connect(switchToRightDocument, SIGNAL(activated()),
            mDocumentManager, SLOT(switchToRightDocument()));
    QShortcut *switchToRightDocument1 = new QShortcut(tr("Ctrl+Tab"), this);
    connect(switchToRightDocument1, SIGNAL(activated()),
            mDocumentManager, SLOT(switchToRightDocument()));

    new QShortcut(tr("X"), this, SLOT(flipStampHorizontally()));
    new QShortcut(tr("Y"), this, SLOT(flipStampVertically()));
    new QShortcut(tr("Z"), this, SLOT(rotateStampRight()));
    new QShortcut(tr("Shift+Z"), this, SLOT(rotateStampLeft()));

    QShortcut *copyPositionShortcut = new QShortcut(tr("Alt+C"), this);
    connect(copyPositionShortcut, SIGNAL(activated()),
            mActionHandler, SLOT(copyPosition()));

    updateActions();
    readSettings();
    setupQuickStamps();

    connect(AutomappingManager::instance(), SIGNAL(warningsOccurred()),
            this, SLOT(autoMappingWarning()));
    connect(AutomappingManager::instance(), SIGNAL(errorsOccurred()),
            this, SLOT(autoMappingError()));
}

MainWindow::~MainWindow()
{
    mDocumentManager->closeAllDocuments();

    AutomappingManager::deleteInstance();
    QuickStampManager::deleteInstance();
    ToolManager::deleteInstance();
    TilesetManager::deleteInstance();
    DocumentManager::deleteInstance();
    Preferences::deleteInstance();
    LanguageManager::deleteInstance();
    PluginManager::deleteInstance();

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

    addMapDocument(mapDocument);
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

    TmxMapReader tmxMapReader;

    if (!mapReader && !tmxMapReader.supportsFile(fileName)) {
        // Try to find a plugin that implements support for this format
        const PluginManager *pm = PluginManager::instance();
        QList<MapReaderInterface*> readers =
                pm->interfaces<MapReaderInterface>();

        foreach (MapReaderInterface *reader, readers) {
            if (reader->supportsFile(fileName)) {
                mapReader = reader;
                break;
            }
        }
    }

    if (!mapReader)
        mapReader = &tmxMapReader;

    Map *map = mapReader->read(fileName);
    if (!map) {
        QMessageBox::critical(this, tr("Error Opening Map"),
                              mapReader->errorString());
        return false;
    }

    addMapDocument(new MapDocument(map, fileName));
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

    selectedFilter = mSettings.value(QLatin1String("lastUsedOpenFilter"),
                                     selectedFilter).toString();

    const PluginManager *pm = PluginManager::instance();
    QList<MapReaderInterface*> readers = pm->interfaces<MapReaderInterface>();
    foreach (const MapReaderInterface *reader, readers) {
        foreach (const QString &str, reader->nameFilters()) {
            if (!str.isEmpty()) {
                filter += QLatin1String(";;");
                filter += str;
            }
        }
    }

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

    mSettings.setValue(QLatin1String("lastUsedOpenFilter"), selectedFilter);
    foreach (const QString &fileName, fileNames)
        openFile(fileName, mapReader);
}

bool MainWindow::saveFile(const QString &fileName)
{
    if (!mMapDocument)
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

    if (currentFileName.endsWith(QLatin1String(".tmx"), Qt::CaseInsensitive))
        return saveFile(currentFileName);
    else
        return saveFileAs();
}

bool MainWindow::saveFileAs()
{
    QString suggestedFileName;
    if (mMapDocument && !mMapDocument->fileName().isEmpty()) {
        const QFileInfo fileInfo(mMapDocument->fileName());
        suggestedFileName = fileInfo.path();
        suggestedFileName += QLatin1Char('/');
        suggestedFileName += fileInfo.completeBaseName();
        suggestedFileName += QLatin1String(".tmx");
    } else {
        suggestedFileName = fileDialogStartLocation();
        suggestedFileName += QLatin1Char('/');
        suggestedFileName += tr("untitled.tmx");
    }

    const QString fileName =
            QFileDialog::getSaveFileName(this, QString(), suggestedFileName,
                                         tr("Tiled map files (*.tmx)"));
    if (!fileName.isEmpty())
        return saveFile(fileName);
    return false;
}

bool MainWindow::confirmSave()
{
    if (!mMapDocument || !mMapDocument->isModified())
        return true;

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
    for (int i = 0; i < mDocumentManager->documentCount(); i++) {
        mDocumentManager->switchToDocument(i);
        if (!confirmSave())
            return false;
    }

    return true;
}

void MainWindow::saveAsImage()
{
    if (!mMapDocument)
        return;

    MapView *mapView = mDocumentManager->currentMapView();
    SaveAsImageDialog dialog(mMapDocument,
                             mMapDocument->fileName(),
                             mapView->zoomable()->scale(),
                             this);
    dialog.exec();
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

    QString selectedFilter =
            mSettings.value(QLatin1String("lastUsedExportFilter")).toString();

    QFileInfo baseNameInfo = QFileInfo(mMapDocument->fileName());
    QString baseName = baseNameInfo.baseName();

    QRegExp extensionFinder(QLatin1String("\\(\\*\\.([^\\)\\s]*)"));
    extensionFinder.indexIn(selectedFilter);
    const QString extension = extensionFinder.cap(1);

    Preferences *pref = Preferences::instance();
    QString lastExportedFilePath = pref->lastPath(Preferences::ExportedFile);

    QString suggestedFilename = lastExportedFilePath
                                + QLatin1String("/") + baseName
                                + QLatin1Char('.') + extension;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export As..."),
                                                    suggestedFilename,
                                                    filter, &selectedFilter);
    if (fileName.isEmpty())
        return;

    pref->setLastPath(Preferences::ExportedFile, QFileInfo(fileName).path());

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

    mSettings.setValue(QLatin1String("lastUsedExportFilter"), selectedFilter);

    if (!chosenWriter->write(mMapDocument->map(), fileName)) {
        QMessageBox::critical(this, tr("Error Saving Map"),
                              chosenWriter->errorString());
    }
}

void MainWindow::closeFile()
{
    if (confirmSave())
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
    const QRegion &tileSelection = mMapDocument->tileSelection();
    const QList<MapObject*> &selectedObjects = mMapDocument->selectedObjects();

    copy();

    QUndoStack *stack = mMapDocument->undoStack();
    stack->beginMacro(tr("Cut"));

    if (tileLayer && !tileSelection.isEmpty()) {
        stack->push(new EraseTiles(mMapDocument, tileLayer, tileSelection));
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

    mClipboardManager->copySelection(mMapDocument);
}

void MainWindow::paste()
{
    if (!mMapDocument)
        return;

    Layer *currentLayer = mMapDocument->currentLayer();
    if (!currentLayer)
        return;

    Map *map = mClipboardManager->map();
    if (!map)
        return;

    // We can currently only handle maps with a single layer
    if (map->layerCount() != 1) {
        // Need to clean up the tilesets since they didn't get an owner
        qDeleteAll(map->tilesets());
        delete map;
        return;
    }

    TilesetManager *tilesetManager = TilesetManager::instance();
    tilesetManager->addReferences(map->tilesets());

    mMapDocument->unifyTilesets(map);
    Layer *layer = map->layerAt(0);

    if (TileLayer *tileLayer = layer->asTileLayer()) {
        // Reset selection and paste into the stamp brush
        mActionHandler->selectNone();
        setStampBrush(tileLayer);
        ToolManager::instance()->selectTool(mStampBrush);
    } else if (ObjectGroup *objectGroup = layer->asObjectGroup()) {
        if (ObjectGroup *currentObjectGroup = currentLayer->asObjectGroup()) {
            // Determine where to insert the objects
            const QPointF center = objectGroup->objectsBoundingRect().center();
            const MapView *view = mDocumentManager->currentMapView();

            // Take the mouse position if the mouse is on the view, otherwise
            // take the center of the view.
            QPoint viewPos;
            if (view->underMouse())
                viewPos = view->mapFromGlobal(QCursor::pos());
            else
                viewPos = QPoint(view->width() / 2, view->height() / 2);

            const MapRenderer *renderer = mMapDocument->renderer();
            const QPointF scenePos = view->mapToScene(viewPos);
            QPointF insertPos = renderer->pixelToTileCoords(scenePos);
            if (Preferences::instance()->snapToGrid())
                insertPos = insertPos.toPoint();
            const QPointF offset = insertPos - center;

            QUndoStack *undoStack = mMapDocument->undoStack();
            QList<MapObject*> pastedObjects;
#if QT_VERSION >= 0x040700
            pastedObjects.reserve(objectGroup->objectCount());
#endif
            undoStack->beginMacro(tr("Paste Objects"));
            foreach (const MapObject *mapObject, objectGroup->objects()) {
                MapObject *objectClone = mapObject->clone();
                objectClone->setPosition(objectClone->position() + offset);
                pastedObjects.append(objectClone);
                undoStack->push(new AddMapObject(mMapDocument,
                                                 currentObjectGroup,
                                                 objectClone));
            }
            undoStack->endMacro();

            mMapDocument->setSelectedObjects(pastedObjects);
        }
    }

    tilesetManager->removeReferences(map->tilesets());
    delete map;
}

void MainWindow::delete_()
{
    if (!mMapDocument)
        return;

    Layer *currentLayer = mMapDocument->currentLayer();
    if (!currentLayer)
        return;

    TileLayer *tileLayer = dynamic_cast<TileLayer*>(currentLayer);
    const QRegion &tileSelection = mMapDocument->tileSelection();
    const QList<MapObject*> &selectedObjects = mMapDocument->selectedObjects();

    QUndoStack *undoStack = mMapDocument->undoStack();
    undoStack->beginMacro(tr("Delete"));

    if (tileLayer && !tileSelection.isEmpty()) {
        undoStack->push(new EraseTiles(mMapDocument, tileLayer, tileSelection));
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

    if (Tileset *tileset = newTileset.createTileset()) {
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

void MainWindow::addExternalTileset()
{
    if (!mMapDocument)
        return;

    const QString start = fileDialogStartLocation();
    const QString fileName =
            QFileDialog::getOpenFileName(this, tr("Add External Tileset"),
                                         start,
                                         tr("Tiled tileset files (*.tsx)"));
    if (fileName.isEmpty())
        return;

    TmxMapReader reader;
    if (Tileset *tileset = reader.readTileset(fileName)) {
        mMapDocument->undoStack()->push(new AddTileset(mMapDocument, tileset));
    } else {
        QMessageBox::critical(this, tr("Error Reading Tileset"),
                              reader.errorString());
    }
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

    MapPropertiesDialog mapPropertiesDialog(mMapDocument, this);

    mapPropertiesDialog.exec();
}

void MainWindow::autoMap()
{
    AutomappingManager::instance()->autoMap();
}

void MainWindow::autoMappingWarning()
{
    const QString title = tr("Automatic Mapping Warning");
    QString warnings = AutomappingManager::instance()->warningString();
    if (!warnings.isEmpty()) {
        QMessageBox::warning(this, title, warnings);
    }
}

void MainWindow::autoMappingError()
{
    const QString title = tr("Automatic Mapping Error");
    QString error = AutomappingManager::instance()->errorString();
    if (!error.isEmpty()) {
        QMessageBox::critical(this, title, error);
    }
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
        selection = mMapDocument->tileSelection();
    }

    const bool canCopy = (tileLayerSelected && !selection.isEmpty())
            || objectsSelected;

    mUi->actionSave->setEnabled(map);
    mUi->actionSaveAs->setEnabled(map);
    mUi->actionSaveAsImage->setEnabled(map);
    mUi->actionExport->setEnabled(map);
    mUi->actionClose->setEnabled(map);
    mUi->actionCloseAll->setEnabled(map);
    mUi->actionCut->setEnabled(canCopy);
    mUi->actionCopy->setEnabled(canCopy);
    mUi->actionPaste->setEnabled(mClipboardManager->hasMap());
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

void MainWindow::editLayerProperties()
{
    if (!mMapDocument)
        return;

    if (Layer *layer = mMapDocument->currentLayer())
        PropertiesDialog::showDialogFor(layer, mMapDocument, this);
}

void MainWindow::flipStampHorizontally()
{
    if (TileLayer *stamp = mStampBrush->stamp()) {
        stamp = static_cast<TileLayer*>(stamp->clone());
        stamp->flip(TileLayer::FlipHorizontally);
        setStampBrush(stamp);
    }
}

void MainWindow::flipStampVertically()
{
    if (TileLayer *stamp = mStampBrush->stamp()) {
        stamp = static_cast<TileLayer*>(stamp->clone());
        stamp->flip(TileLayer::FlipVertically);
        setStampBrush(stamp);
    }
}

void MainWindow::rotateStampLeft()
{
    if (TileLayer *stamp = mStampBrush->stamp()) {
        stamp = static_cast<TileLayer*>(stamp->clone());
        stamp->rotate(TileLayer::RotateLeft);
        setStampBrush(stamp);
    }
}

void MainWindow::rotateStampRight()
{
    if (TileLayer *stamp = mStampBrush->stamp()) {
        stamp = static_cast<TileLayer*>(stamp->clone());
        stamp->rotate(TileLayer::RotateRight);
        setStampBrush(stamp);
    }
}

/**
 * Sets the stamp brush, which is used by both the stamp brush and the bucket
 * fill tool.
 */
void MainWindow::setStampBrush(const TileLayer *tiles)
{
    if (!tiles)
        return;

    mStampBrush->setStamp(static_cast<TileLayer*>(tiles->clone()));
    mBucketFillTool->setStamp(static_cast<TileLayer*>(tiles->clone()));

    // When selecting a new stamp, it makes sense to switch to a stamp tool
    ToolManager *m = ToolManager::instance();
    AbstractTool *selectedTool = m->selectedTool();
    if (selectedTool != mStampBrush && selectedTool != mBucketFillTool)
        m->selectTool(mStampBrush);
}

/**
 * Sets the terrain brush.
 */
void MainWindow::setTerrainBrush(const Terrain *terrain)
{
    mTerrainBrush->setTerrain(terrain);

    // When selecting a new terrain, it makes sense to switch to a terrain brush tool
    ToolManager *m = ToolManager::instance();
    AbstractTool *selectedTool = m->selectedTool();
    if (selectedTool != mTerrainBrush)
        m->selectTool(mTerrainBrush);
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
        mDocumentManager->switchToDocument(i);
        fileList.append(mDocumentManager->currentDocument()->fileName());
        MapView *mapView = mDocumentManager->currentMapView();
        const int currentLayerIndex = mMapDocument->currentLayerIndex();

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
        resize(800, 600);
    restoreState(mSettings.value(QLatin1String("state"),
                                 QByteArray()).toByteArray());
    mSettings.endGroup();
    updateRecentFiles();
}

void MainWindow::updateWindowTitle()
{
    if (mMapDocument) {
        setWindowTitle(tr("[*]%1 - Tiled").arg(mMapDocument->displayName()));
        setWindowFilePath(mMapDocument->fileName());
        setWindowModified(mMapDocument->isModified());
    } else {
        setWindowTitle(QApplication::applicationName());
        setWindowFilePath(QString());
        setWindowModified(false);
    }
}

void MainWindow::addMapDocument(MapDocument *mapDocument)
{
    mDocumentManager->addDocument(mapDocument);

    MapView *mapView = mDocumentManager->currentMapView();
    connect(mapView->zoomable(), SIGNAL(scaleChanged(qreal)),
            this, SLOT(updateZoomLabel()));

    Preferences *prefs = Preferences::instance();

    MapScene *mapScene = mapView->mapScene();
    mapScene->setGridVisible(prefs->showGrid());
    connect(prefs, SIGNAL(showGridChanged(bool)),
            mapScene, SLOT(setGridVisible(bool)));
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
    mActionHandler->retranslateUi();
}

void MainWindow::mapDocumentChanged(MapDocument *mapDocument)
{
    if (mMapDocument)
        mMapDocument->disconnect(this);

    if (mZoomable)
        mZoomable->connectToComboBox(0);
    mZoomable = 0;

    mMapDocument = mapDocument;

    mActionHandler->setMapDocument(mMapDocument);
    mLayerDock->setMapDocument(mMapDocument);
    mObjectsDock->setMapDocument(mMapDocument);
    mTilesetDock->setMapDocument(mMapDocument);
    mTerrainDock->setMapDocument(mMapDocument);
    AutomappingManager::instance()->setMapDocument(mMapDocument);
    QuickStampManager::instance()->setMapDocument(mMapDocument);

    if (mMapDocument) {
        connect(mMapDocument, SIGNAL(fileNameChanged()),
                SLOT(updateWindowTitle()));
        connect(mapDocument, SIGNAL(currentLayerIndexChanged(int)),
                SLOT(updateActions()));
        connect(mapDocument, SIGNAL(tileSelectionChanged(QRegion,QRegion)),
                SLOT(updateActions()));
        connect(mapDocument, SIGNAL(selectedObjectsChanged()),
                SLOT(updateActions()));

        if (MapView *mapView = mDocumentManager->currentMapView()) {
            mZoomable = mapView->zoomable();
            mZoomable->connectToComboBox(mZoomComboBox);
        }
    }

    updateWindowTitle();
    updateActions();
}

void MainWindow::setupQuickStamps()
{
    QuickStampManager *quickStampManager = QuickStampManager::instance();
    QList<int> keys = QuickStampManager::keys();

    QSignalMapper *selectMapper = new QSignalMapper(this);
    QSignalMapper *saveMapper = new QSignalMapper(this);

    for (int i = 0; i < keys.length(); i++) {
        // Set up shortcut for selecting this quick stamp
        QShortcut *selectStamp = new QShortcut(this);
        selectStamp->setKey(keys.value(i));
        connect(selectStamp, SIGNAL(activated()), selectMapper, SLOT(map()));
        selectMapper->setMapping(selectStamp, i);

        // Set up shortcut for saving this quick stamp
        QShortcut *saveStamp = new QShortcut(this);
        saveStamp->setKey(QKeySequence(Qt::CTRL + keys.value(i)));
        connect(saveStamp, SIGNAL(activated()), saveMapper, SLOT(map()));
        saveMapper->setMapping(saveStamp, i);
    }

    connect(selectMapper, SIGNAL(mapped(int)),
            quickStampManager, SLOT(selectQuickStamp(int)));
    connect(saveMapper, SIGNAL(mapped(int)),
            quickStampManager, SLOT(saveQuickStamp(int)));

    connect(quickStampManager, SIGNAL(setStampBrush(const TileLayer*)),
            this, SLOT(setStampBrush(const TileLayer*)));
}

void MainWindow::closeMapDocument(int index)
{
    mDocumentManager->switchToDocument(index);
    if (confirmSave())
        mDocumentManager->closeCurrentDocument();
}
