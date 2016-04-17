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
#include "documentmanager.h"
#include "erasetiles.h"
#include "exportasimagedialog.h"
#include "languagemanager.h"
#include "layer.h"
#include "map.h"
#include "mapdocument.h"
#include "mapdocumentactionhandler.h"
#include "mapeditor.h"
#include "mapformat.h"
#include "mapobject.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "mapview.h"
#include "newmapdialog.h"
#include "newtilesetdialog.h"
#include "pluginmanager.h"
#include "resizedialog.h"
#include "objectgroup.h"
#include "objecttypeseditor.h"
#include "offsetmapdialog.h"
#include "patreondialog.h"
#include "preferences.h"
#include "terrain.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"
#include "tilesetmanager.h"
#include "undodock.h"
#include "utils.h"
#include "zoomable.h"
#include "commandbutton.h"
#include "consoledock.h"
#include "tmxmapformat.h"
#include "tileseteditor.h"
#include "tilesetdocument.h"

#ifdef Q_OS_MAC
#include "macsupport.h"
#endif

#include <QCloseEvent>
#include <QDesktopServices>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QMimeData>
#include <QRegExp>
#include <QSessionManager>
#include <QShortcut>
#include <QTextStream>
#include <QToolButton>
#include <QUndoGroup>
#include <QUndoStack>
#include <QUndoView>

using namespace Tiled;
using namespace Tiled::Internal;
using namespace Tiled::Utils;


MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , mUi(new Ui::MainWindow)
    , mMapDocument(nullptr)
    , mActionHandler(new MapDocumentActionHandler(this))
    , mConsoleDock(new ConsoleDock(this))
    , mObjectTypesEditor(new ObjectTypesEditor(this))
    , mAutomappingManager(new AutomappingManager(this))
    , mDocumentManager(DocumentManager::instance())
    , mTmxMapFormat(new TmxMapFormat(this))
    , mTsxTilesetFormat(new TsxTilesetFormat(this))
{
    mUi->setupUi(this);

    mDocumentManager->setEditor(Document::MapDocumentType, new MapEditor);
    mDocumentManager->setEditor(Document::TilesetDocumentType, new TilesetEditor);

    setCentralWidget(mDocumentManager->widget());

    PluginManager::addObject(mTmxMapFormat);
    PluginManager::addObject(mTsxTilesetFormat);

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

    QUndoGroup *undoGroup = mDocumentManager->undoGroup();
    QAction *undoAction = undoGroup->createUndoAction(this, tr("Undo"));
    QAction *redoAction = undoGroup->createRedoAction(this, tr("Redo"));
    redoAction->setIcon(redoIcon);
    undoAction->setIcon(undoIcon);
    connect(undoGroup, SIGNAL(cleanChanged(bool)), SLOT(updateWindowTitle()));

    UndoDock *undoDock = new UndoDock(undoGroup, this);
    addDockWidget(Qt::BottomDockWidgetArea, mConsoleDock);
    addDockWidget(Qt::LeftDockWidgetArea, undoDock);

//    tabifyDockWidget(undoDock, mMapsDock);
//    tabifyDockWidget(tileStampsDock, undoDock);

    // These dock widgets may not be immediately useful to many people, so
    // they are hidden by default.
    undoDock->setVisible(false);
    mConsoleDock->setVisible(false);

//    mUi->actionNew->setShortcuts(QKeySequence::New);
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
    mUi->actionAutoMapWhileDrawing->setChecked(preferences->automappingDrawing());

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

    // todo: Turn into Command menu
//    mCommandButton = new CommandButton(this);
//    mUi->mainToolBar->addWidget(mCommandButton);

    mUi->menuMap->insertAction(mUi->actionOffsetMap,
                               mActionHandler->actionCropToSelection());

    // todo: Move into Tools menu
//    mRandomButton = new QToolButton(this);
//    mRandomButton->setToolTip(tr("Random Mode"));
//    mRandomButton->setIcon(QIcon(QLatin1String(":images/24x24/dice.png")));
//    mRandomButton->setCheckable(true);
//    mRandomButton->setShortcut(QKeySequence(tr("D")));
//    mUi->mainToolBar->addWidget(mRandomButton);

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

    connect(mUi->actionNewMap, SIGNAL(triggered()), SLOT(newMap()));
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
    connect(mUi->actionAutoMap, SIGNAL(triggered()),
            mAutomappingManager, SLOT(autoMap()));
    connect(mUi->actionAutoMapWhileDrawing, &QAction::toggled,
            preferences, &Preferences::setAutomappingDrawing);
    connect(mUi->actionMapProperties, SIGNAL(triggered()),
            SLOT(editMapProperties()));

    connect(mUi->actionDocumentation, SIGNAL(triggered()), SLOT(openDocumentation()));
    connect(mUi->actionBecomePatron, SIGNAL(triggered()), SLOT(becomePatron()));
    connect(mUi->actionAbout, SIGNAL(triggered()), SLOT(aboutTiled()));

    // todo: figure out how to hook this up
//    connect(mTilesetDock, SIGNAL(tilesetsDropped(QStringList)),
//            SLOT(newTilesets(QStringList)));

    // Add recent file actions to the recent files menu
    for (auto &action : mRecentFiles) {
         action = new QAction(this);
         mUi->menuRecentFiles->insertAction(mUi->actionClearRecentFiles,
                                            action);
         action->setVisible(false);
         connect(action, SIGNAL(triggered()),
                 this, SLOT(openRecentFile()));
    }
    mUi->menuRecentFiles->insertSeparator(mUi->actionClearRecentFiles);

    setThemeIcon(mUi->menuNew, "document-new");
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
//    setThemeIcon(mUi->actionNewTileset, "document-new");
    setThemeIcon(mUi->actionResizeMap, "document-page-setup");
    setThemeIcon(mUi->actionMapProperties, "document-properties");
    setThemeIcon(mUi->actionDocumentation, "help-contents");
    setThemeIcon(mUi->actionAbout, "help-about");



    // todo: Creating a new tileset does not necessarily mean adding it to the
    // current map. Consider automatically figuring out which tilesets are used
    // on a map.
//    connect(mTilesetDock, SIGNAL(newTileset()),
//            this, SLOT(newTileset()));

//    connect(mTerrainDock, &TerrainDock::currentTerrainChanged,
//            mTerrainBrush, &TerrainBrush::setTerrain);
//    connect(mTerrainDock, &TerrainDock::selectTerrainBrush,
//            this, &MainWindow::selectTerrainBrush);
//    connect(mTerrainBrush, &TerrainBrush::terrainCaptured,
//            mTerrainDock, &TerrainDock::setCurrentTerrain);

//    connect(tileStampsDock, SIGNAL(setStamp(TileStamp)),
//            this, SLOT(setStamp(TileStamp)));


    // Add the 'Views and Toolbars' submenu. This needs to happen after all
    // the dock widgets and toolbars have been added to the main window.
    mViewsAndToolbarsMenu = new QAction(tr("Views and Toolbars"), this);
    mShowObjectTypesEditor = new QAction(tr("Object Types Editor"), this);
    mShowObjectTypesEditor->setCheckable(true);
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
    mUi->menuView->insertAction(mUi->actionShowGrid, mShowObjectTypesEditor);
    mUi->menuView->insertAction(mUi->actionShowGrid, mShowTileAnimationEditor);
    mUi->menuView->insertAction(mUi->actionShowGrid, mShowTileCollisionEditor);
    mUi->menuView->insertSeparator(mUi->actionShowGrid);

    connect(mShowObjectTypesEditor, SIGNAL(toggled(bool)),
            mObjectTypesEditor, SLOT(setVisible(bool)));
    connect(mObjectTypesEditor, SIGNAL(closed()), SLOT(onObjectTypesEditorClosed()));

//    connect(mShowTileAnimationEditor, SIGNAL(toggled(bool)),
//            mTileAnimationEditor, SLOT(setVisible(bool)));
//    connect(mTileAnimationEditor, SIGNAL(closed()), SLOT(onAnimationEditorClosed()));

//    connect(mShowTileCollisionEditor, SIGNAL(toggled(bool)),
//            mTileCollisionEditor, SLOT(setVisible(bool)));
//    connect(mTileCollisionEditor, SIGNAL(closed()), SLOT(onCollisionEditorClosed()));

    connect(ClipboardManager::instance(), SIGNAL(hasMapChanged()), SLOT(updateActions()));

    connect(mDocumentManager, SIGNAL(fileOpenRequested(QString)),
            this, SLOT(openFile(QString)));
    connect(mDocumentManager, SIGNAL(fileOpenRequested()),
            this, SLOT(openFile()));
    connect(mDocumentManager, SIGNAL(fileSaveRequested()),
            this, SLOT(saveFile()));
    connect(mDocumentManager, &DocumentManager::currentDocumentChanged,
            this, &MainWindow::mapDocumentChanged);
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

    updateActions();
    readSettings();

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
    // todo: Delete the MapEditor first?
//    mTileAnimationEditor->setTile(nullptr);
//    mTileAnimationEditor->writeSettings();
//    mTileCollisionEditor->setTile(nullptr);
//    mTileCollisionEditor->writeSettings();

    mDocumentManager->deleteEditor(Document::MapDocumentType);
    mDocumentManager->deleteEditor(Document::TilesetDocumentType);

    PluginManager::removeObject(mTmxMapFormat);
    PluginManager::removeObject(mTsxTilesetFormat);

    DocumentManager::deleteInstance();
    TilesetManager::deleteInstance();
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

bool MainWindow::openFile(const QString &fileName, FileFormat *fileFormat)
{
    if (fileName.isEmpty())
        return false;

    // Select existing document if this file is already open
    int documentIndex = mDocumentManager->findDocument(fileName);
    if (documentIndex != -1) {
        mDocumentManager->switchToDocument(documentIndex);
        return true;
    }

    if (!fileFormat) {
        // Try to find a plugin that implements support for this format
        auto formats = PluginManager::objects<FileFormat>();
        for (FileFormat *format : formats) {
            if (format->supportsFile(fileName)) {
                fileFormat = format;
                break;
            }
        }
    }

    if (!fileFormat) {
        QMessageBox::critical(this, tr("Error Opening File"), tr("Unrecognized file format"));
        return false;
    }

    QString error;
    Document *document = nullptr;

    if (MapFormat *mapFormat = qobject_cast<MapFormat*>(fileFormat)) {
        document = MapDocument::load(fileName, mapFormat, &error);
    } else if (TilesetFormat *tilesetFormat = qobject_cast<TilesetFormat*>(fileFormat)) {
        SharedTileset tileset = tilesetFormat->read(fileName);
        if (tileset.isNull())
            error = tilesetFormat->errorString();
        else
            document = new TilesetDocument(tileset, fileName);
    }

    if (!document) {
        QMessageBox::critical(this, tr("Error Opening File"), error);
        return false;
    }

    mDocumentManager->addDocument(document);

    if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document))
        mDocumentManager->checkTilesetColumns(mapDocument);

    setRecentFile(fileName);
    return true;
}

bool MainWindow::openFile(const QString &fileName)
{
    return openFile(fileName, nullptr);
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

    for (int i = 0; i < lastOpenFiles.size(); i++)
        openFile(lastOpenFiles.at(i));

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
    QString selectedFilter = filter;

    FormatHelper<FileFormat> helper(FileFormat::Read, filter);

    selectedFilter = mSettings.value(QLatin1String("lastUsedOpenFilter"),
                                     selectedFilter).toString();

    QStringList fileNames = QFileDialog::getOpenFileNames(this, tr("Open Map"),
                                                          fileDialogStartLocation(),
                                                          helper.filter(),
                                                          &selectedFilter);
    if (fileNames.isEmpty())
        return;

    // When a particular filter was selected, use the associated format
    FileFormat *fileFormat = helper.formatByNameFilter(selectedFilter);

    mSettings.setValue(QLatin1String("lastUsedOpenFilter"), selectedFilter);
    foreach (const QString &fileName, fileNames)
        openFile(fileName, fileFormat);
}

/**
 * Save the given document with the given file name. When saved
 * successfully, the file is added to the list of recent files.
 *
 * @return <code>true</code> on success, <code>false</code> on failure
 */
bool MainWindow::saveDocument(Document *document, const QString &fileName)
{
    if (fileName.isEmpty())
        return false;

    QString error;
    if (!document->save(fileName, &error)) {
        QMessageBox::critical(this, tr("Error Saving File"), error);
        return false;
    }

    setRecentFile(fileName);
    return true;
}

static Document *saveAsDocument(Document *document)
{
    if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document))
        if (tilesetDocument->isEmbedded())
            document = tilesetDocument->mapDocuments().first();

    return document;
}

bool MainWindow::saveFile()
{
    Document *document = mDocumentManager->currentDocument();
    if (!document)
        return false;

    document = saveAsDocument(document);

    const QString currentFileName = document->fileName();

    if (currentFileName.isEmpty())
        return saveDocumentAs(document);
    else
        return saveDocument(document, currentFileName);
}

bool MainWindow::saveFileAs()
{
    Document *document = mDocumentManager->currentDocument();
    if (!document)
        return false;

    document = saveAsDocument(document);

    return saveDocumentAs(document);
}

/**
 * Save the given document with a file name chosen by the user. When saved
 * successfully, the file is added to the list of recent files.
 *
 * @return <code>true</code> on success, <code>false</code> on failure
 */
bool MainWindow::saveDocumentAs(Document *document)
{
    QString filter;
    QString selectedFilter;
    QString fileName = document->fileName();

    if (FileFormat *format = document->writerFormat())
        selectedFilter = format->nameFilter();

    auto getSaveFileName = [&,this](const QString &defaultFileName) {
        if (fileName.isEmpty()) {
            fileName = fileDialogStartLocation();
            fileName += QLatin1Char('/');
            fileName += defaultFileName;
        }

        return QFileDialog::getSaveFileName(this, QString(),
                                            fileName,
                                            filter,
                                            &selectedFilter);
    };

    if (auto mapDocument = qobject_cast<MapDocument*>(document)) {
        if (selectedFilter.isEmpty())
            selectedFilter = TmxMapFormat().nameFilter();

        FormatHelper<MapFormat> helper(FileFormat::ReadWrite);
        filter = helper.filter();

        fileName = getSaveFileName(tr("untitled.tmx"));
        if (fileName.isEmpty())
            return false;

        MapFormat *format = helper.formatByNameFilter(selectedFilter);
        mapDocument->setWriterFormat(format);

    } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
        if (selectedFilter.isEmpty())
            selectedFilter = TsxTilesetFormat().nameFilter();

        FormatHelper<TilesetFormat> helper(FileFormat::ReadWrite);
        filter = helper.filter();

        fileName = getSaveFileName(tr("untitled.tsx"));
        if (fileName.isEmpty())
            return false;

        TilesetFormat *format = helper.formatByNameFilter(selectedFilter);
        tilesetDocument->setWriterFormat(format);
    }

    return saveDocument(document, fileName);
}

bool isEmbeddedTilesetDocument(Document *document)
{
    if (auto *tilesetDocument = qobject_cast<TilesetDocument*>(document))
        return tilesetDocument->isEmbedded();
    return false;
}

void MainWindow::saveAll()
{
    for (Document *document : mDocumentManager->documents()) {
        if (!mDocumentManager->isDocumentModified(document))
            continue;

        // Skip embedded tilesets, they will be saved when their map is checked
        if (isEmbeddedTilesetDocument((document)))
            continue;

        QString fileName(document->fileName());
        QString error;

        if (fileName.isEmpty()) {
            mDocumentManager->switchToDocument(document);
            if (!saveDocumentAs(document))
                return;
        } else if (!document->save(fileName, &error)) {
            mDocumentManager->switchToDocument(document);
            QMessageBox::critical(this, tr("Error Saving File"), error);
            return;
        }

        setRecentFile(fileName);
    }
}

bool MainWindow::confirmSave(Document *document)
{
    if (!document || !mDocumentManager->isDocumentModified(document))
        return true;

    mDocumentManager->switchToDocument(document);

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
    for (Document *document : mDocumentManager->documents()) {
        if (isEmbeddedTilesetDocument((document)))
            continue;
        if (!confirmSave(document))
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
//            statusBar()->showMessage(tr("Exported to %1").arg(exportFileName),
//                                     3000);
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
//        Map *stamp = map.take(); // TileStamp will take ownership
//        setStamp(TileStamp(stamp));
//        tilesetManager->removeReferences(stamp->tilesets());
//        mToolManager->selectTool(mStampBrush);
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
    if (!mPreferencesDialog) {
        mPreferencesDialog = new PreferencesDialog(this);
        mPreferencesDialog->setAttribute(Qt::WA_DeleteOnClose);
    }

    mPreferencesDialog->show();
    mPreferencesDialog->activateWindow();
    mPreferencesDialog->raise();
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
    Preferences *prefs = Preferences::instance();

    const QString startLocation = path.isEmpty()
            ? QFileInfo(prefs->lastPath(Preferences::ImageFile)).absolutePath()
            : path;

    NewTilesetDialog newTileset(this);
    newTileset.setImagePath(startLocation);
//    newTileset.setTileSize(map->tileSize());

    if (SharedTileset tileset = newTileset.createTileset()) {
        mDocumentManager->addDocument(new TilesetDocument(tileset));

//        mMapDocument->undoStack()->push(new AddTileset(mMapDocument, tileset));
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

    QString filter = tr("All Files (*)");

    QString selectedFilter = TsxTilesetFormat().nameFilter();
    filter += QLatin1String(";;");
    filter += selectedFilter;

    FormatHelper<TilesetFormat> helper(FileFormat::Read, filter);

    selectedFilter = mSettings.value(QLatin1String("lastUsedTilesetFilter"),
                                     selectedFilter).toString();

    Preferences *prefs = Preferences::instance();
    QString start = prefs->lastPath(Preferences::ExternalTileset);

    const QStringList fileNames =
            QFileDialog::getOpenFileNames(this, tr("Add External Tileset(s)"),
                                          start,
                                          helper.filter(),
                                          &selectedFilter);

    if (fileNames.isEmpty())
        return;

    prefs->setLastPath(Preferences::ExternalTileset,
                       QFileInfo(fileNames.last()).path());

    mSettings.setValue(QLatin1String("lastUsedTilesetFilter"), selectedFilter);

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
    for (const SharedTileset &tileset : tilesets)
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
    emit mMapDocument->editCurrentObject();
}

void MainWindow::autoMappingError(bool automatic)
{
    const QString title = tr("Automatic Mapping Error");
    QString error = mAutomappingManager->errorString();
    if (!error.isEmpty()) {
        if (automatic)
            ;//statusBar()->showMessage(error, 3000);
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
            ;//statusBar()->showMessage(warning, 3000);
        else
            QMessageBox::warning(this, title, warning);
    }
}

void MainWindow::onObjectTypesEditorClosed()
{
    mShowObjectTypesEditor->setChecked(false);
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
    Document *document = mDocumentManager->currentDocument();
    bool tileLayerSelected = false;
    bool objectsSelected = false;
    QRegion selection;

    if (mMapDocument) {
        Layer *currentLayer = mMapDocument->currentLayer();

        tileLayerSelected = dynamic_cast<TileLayer*>(currentLayer) != nullptr;
        objectsSelected = !mMapDocument->selectedObjects().isEmpty();
        selection = mMapDocument->selectedArea();
    }

    const bool canCopy = (tileLayerSelected && !selection.isEmpty())
            || objectsSelected;

    mUi->actionSave->setEnabled(document);
    mUi->actionSaveAs->setEnabled(document);
    mUi->actionSaveAll->setEnabled(document);

    mUi->actionExportAsImage->setEnabled(mMapDocument);
    mUi->actionExport->setEnabled(mMapDocument);
    mUi->actionExportAs->setEnabled(mMapDocument);
    mUi->actionReload->setEnabled(mMapDocument);
    mUi->actionClose->setEnabled(document);
    mUi->actionCloseAll->setEnabled(document);

    mUi->actionCut->setEnabled(canCopy);
    mUi->actionCopy->setEnabled(canCopy);
    mUi->actionPaste->setEnabled(ClipboardManager::instance()->hasMap());
    mUi->actionDelete->setEnabled(canCopy);

//    mUi->actionNewTileset->setEnabled(mMapDocument);
    mUi->actionAddExternalTileset->setEnabled(mMapDocument);
    mUi->actionResizeMap->setEnabled(mMapDocument);
    mUi->actionOffsetMap->setEnabled(mMapDocument);
    mUi->actionMapProperties->setEnabled(mMapDocument);
    mUi->actionAutoMap->setEnabled(mMapDocument);

//    mCommandButton->setEnabled(mMapDocument);

    updateZoomLabel(); // for the zoom actions
}

void MainWindow::updateZoomLabel()
{
    MapView *mapView = mDocumentManager->currentMapView();

    Zoomable *zoomable = mapView ? mapView->zoomable() : nullptr;
    const qreal scale = zoomable ? zoomable->scale() : 1;

    mUi->actionZoomIn->setEnabled(zoomable && zoomable->canZoomIn());
    mUi->actionZoomOut->setEnabled(zoomable && zoomable->canZoomOut());
    mUi->actionZoomNormal->setEnabled(scale != 1);
}

void MainWindow::openDocumentation()
{
    QDesktopServices::openUrl(QUrl(QLatin1String("http://doc.mapeditor.org")));
}

void MainWindow::writeSettings()
{
    mSettings.beginGroup(QLatin1String("mainwindow"));
    mSettings.setValue(QLatin1String("geometry"), saveGeometry());
    mSettings.setValue(QLatin1String("state"), saveState());
    mSettings.endGroup();

    mSettings.beginGroup(QLatin1String("recentFiles"));
    if (Document *document = mDocumentManager->currentDocument())
        mSettings.setValue(QLatin1String("lastActive"), document->fileName());

    QStringList fileList;
    for (int i = 0; i < mDocumentManager->documentCount(); i++) {
        Document *document = mDocumentManager->documents().at(i);
        fileList.append(document->fileName());
    }
    mSettings.setValue(QLatin1String("lastOpenFiles"), fileList);
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

//    mRandomButton->setToolTip(tr("Random Mode"));
    mLayerMenu->setTitle(tr("&Layer"));
    mViewsAndToolbarsMenu->setText(tr("Views and Toolbars"));
    mShowTileAnimationEditor->setText(tr("Tile Animation Editor"));
    mShowTileCollisionEditor->setText(tr("Tile Collision Editor"));
    mActionHandler->retranslateUi();
}

void MainWindow::mapDocumentChanged(Document *document)
{
    MapDocument *mapDocument = qobject_cast<MapDocument*>(document);

    // todo: most of this code will have to be moved to the MapEditor

    if (mMapDocument)
        mMapDocument->disconnect(this);

    mMapDocument = mapDocument;

    mActionHandler->setMapDocument(mapDocument);
    mAutomappingManager->setMapDocument(mapDocument);

    if (document) {
        connect(document, SIGNAL(fileNameChanged(QString,QString)),
                SLOT(updateWindowTitle()));
    }

    // todo: adapt updateActions or find new ways to get the actions updated
//        connect(mapDocument, SIGNAL(currentLayerIndexChanged(int)),
//                SLOT(updateActions()));
//        connect(mapDocument, SIGNAL(selectedAreaChanged(QRegion,QRegion)),
//                SLOT(updateActions()));
//        connect(mapDocument, SIGNAL(selectedObjectsChanged()),
//                SLOT(updateActions()));

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
