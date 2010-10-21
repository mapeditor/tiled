/*
 * mainwindow.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2009-2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2009, Dennis Honeyman <arcticuno@gmail.com>
 * Copyright 2009, Christian Henz <chrhenz@gmx.de>
 * Copyright 2010, Andrew G. Crowell <overkill9999@gmail.com>
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
#include "automap.h"
#include "addremovetileset.h"
#include "clipboardmanager.h"
#include "createobjecttool.h"
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
#include "mapscene.h"
#include "newmapdialog.h"
#include "newtilesetdialog.h"
#include "pluginmanager.h"
#include "propertiesdialog.h"
#include "resizedialog.h"
#include "offsetmapdialog.h"
#include "preferences.h"
#include "preferencesdialog.h"
#include "saveasimagedialog.h"
#include "selectiontool.h"
#include "stampbrush.h"
#include "tilelayer.h"
#include "tileset.h"
#include "tilesetdock.h"
#include "tilesetmanager.h"
#include "toolmanager.h"
#include "tmxmapreader.h"
#include "tmxmapwriter.h"
#include "undodock.h"
#include "utils.h"
#include "zoomable.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QSessionManager>
#include <QTextStream>
#include <QUndoGroup>
#include <QUndoStack>
#include <QUndoView>
#include <QImageReader>
#include <QSignalMapper>
#include <QShortcut>

using namespace Tiled;
using namespace Tiled::Internal;
using namespace Tiled::Utils;

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
    , mUi(new Ui::MainWindow)
    , mMapDocument(0)
    , mActionHandler(new MapDocumentActionHandler(this))
    , mLayerDock(new LayerDock(this))
    , mTilesetDock(new TilesetDock(this))
    , mZoomLabel(new QLabel)
    , mStatusInfoLabel(new QLabel)
    , mClipboardManager(new ClipboardManager(this))
{
    mUi->setupUi(this);

    PluginManager::instance()->loadPlugins();

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

    mUndoGroup = new QUndoGroup(this);
    QAction *undoAction = mUndoGroup->createUndoAction(this, tr("Undo"));
    QAction *redoAction = mUndoGroup->createRedoAction(this, tr("Redo"));
    redoAction->setIcon(redoIcon);
    undoAction->setIcon(undoIcon);
    connect(mUndoGroup, SIGNAL(cleanChanged(bool)), SLOT(updateModified()));

    UndoDock *undoDock = new UndoDock(mUndoGroup, this);

    addDockWidget(Qt::RightDockWidgetArea, mLayerDock);
    addDockWidget(Qt::RightDockWidgetArea, undoDock);
    tabifyDockWidget(undoDock, mLayerDock);
    addDockWidget(Qt::RightDockWidgetArea, mTilesetDock);

    updateZoomLabel(mUi->mapView->zoomable()->scale());
    connect(mUi->mapView->zoomable(), SIGNAL(scaleChanged(qreal)),
            this, SLOT(updateZoomLabel(qreal)));

    statusBar()->addPermanentWidget(mZoomLabel);

    mUi->actionNew->setShortcuts(QKeySequence::New);
    mUi->actionOpen->setShortcuts(QKeySequence::Open);
    mUi->actionSave->setShortcuts(QKeySequence::Save);
    mUi->actionSaveAs->setShortcuts(QKeySequence::SaveAs);
    mUi->actionClose->setShortcuts(QKeySequence::Close);
#if QT_VERSION >= 0x040600
    mUi->actionQuit->setShortcuts(QKeySequence::Quit);
#endif
    mUi->actionCut->setShortcuts(QKeySequence::Cut);
    mUi->actionCopy->setShortcuts(QKeySequence::Copy);
    mUi->actionPaste->setShortcuts(QKeySequence::Paste);
    undoAction->setShortcuts(QKeySequence::Undo);
    redoAction->setShortcuts(QKeySequence::Redo);

    // Make sure Ctrl+= also works for zooming in
    QList<QKeySequence> keys = QKeySequence::keyBindings(QKeySequence::ZoomIn);
    keys += QKeySequence(tr("Ctrl+="));
    mUi->actionZoomIn->setShortcuts(keys);
    mUi->actionZoomOut->setShortcuts(QKeySequence::ZoomOut);

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

    mLayerMenu = new QMenu(tr("&Layer"), this);
    mLayerMenu->addAction(mActionHandler->actionAddTileLayer());
    mLayerMenu->addAction(mActionHandler->actionAddObjectGroup());
    mLayerMenu->addAction(mActionHandler->actionDuplicateLayer());
    mLayerMenu->addAction(mActionHandler->actionRemoveLayer());
    mLayerMenu->addSeparator();
    mLayerMenu->addAction(mActionHandler->actionMoveLayerUp());
    mLayerMenu->addAction(mActionHandler->actionMoveLayerDown());
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
    connect(mUi->actionQuit, SIGNAL(triggered()), SLOT(close()));

    connect(mUi->actionCut, SIGNAL(triggered()), SLOT(cut()));
    connect(mUi->actionCopy, SIGNAL(triggered()), SLOT(copy()));
    connect(mUi->actionPaste, SIGNAL(triggered()), SLOT(paste()));
    connect(mUi->actionPreferences, SIGNAL(triggered()),
            SLOT(openPreferences()));
    connect(mUi->actionAutoMap, SIGNAL(triggered()), SLOT(autoMap()));

    connect(mUi->actionZoomIn, SIGNAL(triggered()),
            mUi->mapView->zoomable(), SLOT(zoomIn()));
    connect(mUi->actionZoomOut, SIGNAL(triggered()),
            mUi->mapView->zoomable(), SLOT(zoomOut()));
    connect(mUi->actionZoomNormal, SIGNAL(triggered()),
            mUi->mapView->zoomable(), SLOT(resetZoom()));

    connect(mUi->actionNewTileset, SIGNAL(triggered()), SLOT(newTileset()));
    connect(mUi->actionAddExternalTileset, SIGNAL(triggered()),
            SLOT(addExternalTileset()));
    connect(mUi->actionResizeMap, SIGNAL(triggered()), SLOT(resizeMap()));
    connect(mUi->actionOffsetMap, SIGNAL(triggered()), SLOT(offsetMap()));
    connect(mUi->actionMapProperties, SIGNAL(triggered()),
            SLOT(editMapProperties()));

    connect(mActionHandler->actionLayerProperties(), SIGNAL(triggered()),
            SLOT(editLayerProperties()));

    connect(mUi->actionAbout, SIGNAL(triggered()), SLOT(aboutTiled()));
    connect(mUi->actionAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

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
    setThemeIcon(redoAction, "edit-redo");
    setThemeIcon(undoAction, "edit-undo");
    setThemeIcon(mUi->actionZoomIn, "zoom-in");
    setThemeIcon(mUi->actionZoomOut, "zoom-out");
    setThemeIcon(mUi->actionZoomNormal, "zoom-original");
    setThemeIcon(mUi->actionNewTileset, "document-new");
    setThemeIcon(mUi->actionResizeMap, "document-page-setup");
    setThemeIcon(mUi->actionMapProperties, "document-properties");
    setThemeIcon(mUi->actionAbout, "help-about");

    mScene = new MapScene(this);
    mUi->mapView->setScene(mScene);
    mUi->mapView->centerOn(0, 0);
#ifdef Q_OS_MAC
    mUi->mapView->setFrameStyle(QFrame::NoFrame);
#endif

    mUi->actionShowGrid->setChecked(mScene->isGridVisible());
    connect(mUi->actionShowGrid, SIGNAL(toggled(bool)),
            mScene, SLOT(setGridVisible(bool)));

    mStampBrush = new StampBrush(this);
    mBucketFillTool = new BucketFillTool(this);
    CreateObjectTool *createTileObjectsTool =
            new CreateObjectTool(CreateObjectTool::TileObjects, this);

    connect(mTilesetDock, SIGNAL(currentTilesChanged(const TileLayer*)),
            this, SLOT(setStampBrush(const TileLayer*)));
    connect(mStampBrush, SIGNAL(currentTilesChanged(const TileLayer*)),
            this, SLOT(setStampBrush(const TileLayer*)));
    connect(mTilesetDock, SIGNAL(currentTileChanged(Tile*)),
            createTileObjectsTool, SLOT(setTile(Tile*)));

    ToolManager *toolManager = ToolManager::instance();
    toolManager->registerTool(mStampBrush);
    toolManager->registerTool(mBucketFillTool);
    toolManager->registerTool(new Eraser(this));
    toolManager->registerTool(new SelectionTool(this));
    toolManager->registerTool(
                new CreateObjectTool(CreateObjectTool::AreaObjects, this));
    toolManager->registerTool(createTileObjectsTool);

    addToolBar(toolManager->toolBar());

    statusBar()->addWidget(mStatusInfoLabel);
    connect(toolManager, SIGNAL(statusInfoChanged(QString)),
            this, SLOT(updateStatusInfoLabel(QString)));

    mUi->menuView->addSeparator();
    mUi->menuView->addAction(mTilesetDock->toggleViewAction());
    mUi->menuView->addAction(mLayerDock->toggleViewAction());
    mUi->menuView->addAction(undoDock->toggleViewAction());

    connect(mClipboardManager, SIGNAL(hasMapChanged()), SLOT(updateActions()));

    updateActions();
    readSettings();
    setupQuickStamps();
}

MainWindow::~MainWindow()
{
    writeSettings();

    setMapDocument(0);
    cleanQuickStamps();

    ToolManager::deleteInstance();
    TilesetManager::deleteInstance();
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
        if (!confirmSave())
            manager.cancel();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (confirmSave())
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

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    const QList<QUrl> urls = e->mimeData()->urls();
    if (urls.size() == 1 && !urls.at(0).toLocalFile().isEmpty())
        e->accept();
}

void MainWindow::dropEvent(QDropEvent *e)
{
    const QString file = e->mimeData()->urls().at(0).toLocalFile();
    const QString extension = QFileInfo(file).suffix();

    // Treat file as a tileset if it is an image. Use the extension here because
    // QImageReader::imageFormat() treats tmx files as svg
    const QList<QByteArray> formats = QImageReader::supportedImageFormats();
    foreach (const QByteArray &format, formats) {
        if (extension.compare(QString::fromLatin1(format), Qt::CaseInsensitive) == 0) {
            newTileset(file);
            return;
        }
    }

    // Treat file as a map otherwise
    openFile(file);
}

void MainWindow::newMap()
{
    if (!confirmSave())
        return;

    NewMapDialog newMapDialog(this);
    MapDocument *mapDocument = newMapDialog.createMap();

    if (!mapDocument)
        return;

    setMapDocument(mapDocument);
    mUi->mapView->centerOn(0, 0);

    updateActions();
}

bool MainWindow::openFile(const QString &fileName,
                          MapReaderInterface *mapReader)
{
    if (fileName.isEmpty() || !confirmSave())
        return false;

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

    setMapDocument(new MapDocument(map, fileName));
    mUi->mapView->centerOn(0, 0);

    updateActions();
    return true;
}

void MainWindow::openLastFile()
{
    const QStringList files = recentFiles();

    if (!files.isEmpty() && openFile(files.first())) {
        // Restore camera to the previous position
        mSettings.beginGroup(QLatin1String("mainwindow"));
        qreal scale = mSettings.value(QLatin1String("mapScale")).toDouble();
        if (scale > 0)
            mUi->mapView->zoomable()->setScale(scale);

        const int hor = mSettings.value(QLatin1String("scrollX")).toInt();
        const int ver = mSettings.value(QLatin1String("scrollY")).toInt();
        mUi->mapView->horizontalScrollBar()->setSliderPosition(hor);
        mUi->mapView->verticalScrollBar()->setSliderPosition(ver);

        int layer = mSettings.value(QLatin1String("selectedLayer")).toInt();
        if (layer > 0 && layer < mMapDocument->map()->layerCount())
            mMapDocument->setCurrentLayer(layer);

        mSettings.endGroup();
    }
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
    foreach (MapReaderInterface *reader, readers) {
        filter += QLatin1String(";;");
        filter += reader->nameFilter();
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Map"),
                                                    fileDialogStartLocation(),
                                                    filter, &selectedFilter);
    if (fileName.isEmpty())
        return;

    // When a particular filter was selected, use the associated reader
    MapReaderInterface *mapReader = 0;
    foreach (MapReaderInterface *reader, readers) {
        if (selectedFilter == reader->nameFilter())
            mapReader = reader;
    }

    mSettings.setValue(QLatin1String("lastUsedOpenFilter"), selectedFilter);
    openFile(fileName, mapReader);
}

bool MainWindow::saveFile(const QString &fileName)
{
    if (!mMapDocument)
        return false;

    TmxMapWriter mapWriter;

    if (!mapWriter.write(mMapDocument->map(), fileName)) {
        QMessageBox::critical(this, tr("Error Saving Map"),
                              mapWriter.errorString());
        return false;
    }

    mMapDocument->undoStack()->setClean();
    mMapDocument->setFileName(fileName);
    setCurrentFileName(fileName);
    return true;
}

bool MainWindow::saveFile()
{
    if (mCurrentFileName.endsWith(QLatin1String(".tmx"), Qt::CaseInsensitive))
        return saveFile(mCurrentFileName);
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
    if (!mMapDocument || mMapDocument->undoStack()->isClean())
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

void MainWindow::saveAsImage()
{
    if (!mMapDocument)
        return;

    SaveAsImageDialog dialog(mMapDocument,
                             mCurrentFileName,
                             mUi->mapView->zoomable()->scale(),
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
    foreach (MapWriterInterface *writer, writers) {
        filter += QLatin1String(";;");
        filter += writer->nameFilter();
    }

    QString selectedFilter =
            mSettings.value(QLatin1String("lastUsedExportFilter")).toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export As..."),
                                                    fileDialogStartLocation(),
                                                    filter, &selectedFilter);
    if (fileName.isEmpty())
        return;

    MapWriterInterface *chosenWriter = 0;

    // If a specific filter was selected, use that writer
    foreach (MapWriterInterface *writer, writers)
        if (writer->nameFilter() == selectedFilter)
            chosenWriter = writer;

    // If not, try to find the file extension among the name filters
    QString suffix = QFileInfo(fileName).completeSuffix();
    if (!chosenWriter && !suffix.isEmpty()) {
        suffix.prepend(QLatin1String("*."));
        foreach (MapWriterInterface *writer, writers)
            if (writer->nameFilter().contains(suffix, Qt::CaseInsensitive))
                chosenWriter = writer;
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
    if (confirmSave()) {
        setMapDocument(0);
        updateActions();
    }
}

void MainWindow::cut()
{
    if (!mMapDocument)
        return;

    int currentLayer = mMapDocument->currentLayer();
    if (currentLayer == -1)
        return;

    Map *map = mMapDocument->map();
    Layer *layer = map->layerAt(currentLayer);
    TileLayer *tileLayer = dynamic_cast<TileLayer*>(layer);
    if (!tileLayer)
        return;

    const QRegion &selection = mMapDocument->tileSelection();
    if (selection.isEmpty())
        return;

    copy();

    QUndoStack *stack = mMapDocument->undoStack();
    stack->beginMacro(tr("Cut"));
    stack->push(new EraseTiles(mMapDocument, tileLayer, selection));
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

    Map *map = mClipboardManager->map();
    if (!map)
        return;

    // We can currently only handle maps with a single tile layer
    if (!(map->layerCount() == 1 && map->layerAt(0)->asTileLayer())) {
        // Need to clean up the tilesets since they didn't get an owner
        qDeleteAll(map->tilesets());
        delete map;
        return;
    }

    mMapDocument->unifyTilesets(map);

    TileLayer *tileLayer = map->layerAt(0)->asTileLayer();

    // Reset selection and paste into the stamp brush
    mActionHandler->selectNone();
    setStampBrush(tileLayer);
    ToolManager::instance()->selectTool(mStampBrush);

    delete map;
}

void MainWindow::openPreferences()
{
    PreferencesDialog preferencesDialog(this);
    preferencesDialog.exec();
}

void MainWindow::newTileset(const QString &path)
{
    if (!mMapDocument)
        return;

    Map *map = mMapDocument->map();

    QString startLocation = path.isEmpty()
                            ? fileDialogStartLocation()
                            : path;

    NewTilesetDialog newTileset(startLocation, this);
    newTileset.setTileWidth(map->tileWidth());
    newTileset.setTileHeight(map->tileHeight());

    if (Tileset *tileset = newTileset.createTileset())
        mMapDocument->undoStack()->push(new AddTileset(mMapDocument, tileset));
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
    PropertiesDialog propertiesDialog(tr("Map"),
                                      mMapDocument->map(),
                                      mMapDocument->undoStack(),
                                      this);
    propertiesDialog.exec();
}

void MainWindow::autoMap()
{
    if (!mMapDocument)
        return;

    const QString mapPath = QFileInfo(mMapDocument->fileName()).path();
    const QString rulesFileName = mapPath + QLatin1String("/rules.txt");
    QFile rulesFile(rulesFileName);

    if (!rulesFile.exists()) {
        QMessageBox::critical(
                    this, tr("AutoMap Error"),
                    tr("No rules file found at:\n%1").arg(rulesFileName));
        return;
    }
    if (!rulesFile.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(
                    this, tr("AutoMap Error"),
                    tr("Error opening rules file:\n%1").arg(rulesFileName));
        return;
    }

    QTextStream in(&rulesFile);
    QString line = in.readLine();

    for (; !line.isNull(); line = in.readLine()) {
        QString rulePath = line.trimmed();
        if (rulePath.isEmpty()
                || rulePath.startsWith(QLatin1Char('#'))
                || rulePath.startsWith(QLatin1String("//")))
            continue;

        if (QFileInfo(rulePath).isRelative())
            rulePath = mapPath + QLatin1Char('/') + rulePath;

        if (!QFileInfo(rulePath).exists()) {
            QMessageBox::warning(
                        this, tr("AutoMap Warning"),
                        tr("Rules map not found:\n%1").arg(rulePath));
            continue;
        }

        TmxMapReader mapReader;
        Map *rules = mapReader.read(rulePath);

        QUndoStack *undoStack = mMapDocument->undoStack();
        undoStack->beginMacro(tr("AutoMap: apply ruleset: ") + rulePath);
        undoStack->push(new AutomaticMapping(mMapDocument, rules));
        undoStack->endMacro();

        delete rules;
    }
}

void MainWindow::updateModified()
{
    setWindowModified(!mUndoGroup->isClean());
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        openFile(action->data().toString());
}

QStringList MainWindow::recentFiles() const
{
    return mSettings.value(QLatin1String("recentFiles")).toStringList();
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

    mSettings.setValue(QLatin1String("recentFiles"), files);
    updateRecentFiles();
}

void MainWindow::clearRecentFiles()
{
    mSettings.setValue(QLatin1String("recentFiles"), QStringList());
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
    int currentLayer = -1;
    bool tileLayerSelected = false;
    QRegion selection;

    if (mMapDocument) {
        map = mMapDocument->map();
        currentLayer = mMapDocument->currentLayer();
        selection = mMapDocument->tileSelection();

        if (currentLayer != -1) {
            Layer *layer = mMapDocument->map()->layerAt(currentLayer);
            tileLayerSelected = dynamic_cast<TileLayer*>(layer) != 0;
        }
    }

    const bool mapInClipboard = mClipboardManager->hasMap();

    mUi->actionSave->setEnabled(map);
    mUi->actionSaveAs->setEnabled(map);
    mUi->actionSaveAsImage->setEnabled(map);
    mUi->actionExport->setEnabled(map);
    mUi->actionClose->setEnabled(map);
    mUi->actionCut->setEnabled(tileLayerSelected && !selection.isEmpty());
    mUi->actionCopy->setEnabled(tileLayerSelected && !selection.isEmpty());
    mUi->actionPaste->setEnabled(tileLayerSelected && mapInClipboard);
    mUi->actionNewTileset->setEnabled(map);
    mUi->actionAddExternalTileset->setEnabled(map);
    mUi->actionResizeMap->setEnabled(map);
    mUi->actionOffsetMap->setEnabled(map);
    mUi->actionMapProperties->setEnabled(map);
    mUi->actionAutoMap->setEnabled(map);
}

void MainWindow::updateZoomLabel(qreal scale)
{
    const Zoomable *zoomable = mUi->mapView->zoomable();
    mUi->actionZoomIn->setEnabled(zoomable->canZoomIn());
    mUi->actionZoomOut->setEnabled(zoomable->canZoomOut());
    mUi->actionZoomNormal->setEnabled(scale != 1);

    mZoomLabel->setText(tr("%1%").arg(scale * 100));
}

void MainWindow::editLayerProperties()
{
    if (!mMapDocument)
        return;

    const int layerIndex = mMapDocument->currentLayer();
    if (layerIndex == -1)
        return;

    Layer *layer = mMapDocument->map()->layerAt(layerIndex);
    PropertiesDialog::showDialogFor(layer, mMapDocument, this);
}

/**
 * Sets the stamp brush in response to a change in the selection in the tileset
 * view.
 */
void MainWindow::setStampBrush(const TileLayer *tiles)
{
    if (tiles) {
        mStampBrush->setStamp(static_cast<TileLayer*>(tiles->clone()));
        mBucketFillTool->setStamp(static_cast<TileLayer*>(tiles->clone()));
    }
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
    mSettings.setValue(QLatin1String("gridVisible"),
                       mUi->actionShowGrid->isChecked());
    mSettings.setValue(QLatin1String("mapScale"),
                       mUi->mapView->zoomable()->scale());
    mSettings.setValue(QLatin1String("scrollX"),
                       mUi->mapView->horizontalScrollBar()->sliderPosition());
    mSettings.setValue(QLatin1String("scrollY"),
                       mUi->mapView->verticalScrollBar()->sliderPosition());
    if (mMapDocument)
        mSettings.setValue(QLatin1String("selectedLayer"),
                           mMapDocument->currentLayer());
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
    mUi->actionShowGrid->setChecked(
            mSettings.value(QLatin1String("gridVisible"), true).toBool());
    mSettings.endGroup();
    updateRecentFiles();
}

void MainWindow::setCurrentFileName(const QString &fileName)
{
    mCurrentFileName = fileName;
    setWindowFilePath(mCurrentFileName);
    setWindowTitle(tr("%1[*] - Tiled").arg(QFileInfo(fileName).fileName()));
    setRecentFile(mCurrentFileName);
}

void MainWindow::setMapDocument(MapDocument *mapDocument)
{
    if (mMapDocument)
        mUndoGroup->removeStack(mMapDocument->undoStack());

    mActionHandler->setMapDocument(mapDocument);
    mScene->setMapDocument(mapDocument);
    mLayerDock->setMapDocument(mapDocument);
    mTilesetDock->setMapDocument(mapDocument);

    // TODO: Add support for multiple map documents
    delete mMapDocument;
    mMapDocument = mapDocument;

    if (mMapDocument) {
        setCurrentFileName(mMapDocument->fileName());

        connect(mapDocument, SIGNAL(currentLayerChanged(int)),
                SLOT(updateActions()));
        connect(mapDocument, SIGNAL(tileSelectionChanged(QRegion,QRegion)),
                SLOT(updateActions()));

        QUndoStack *undoStack = mMapDocument->undoStack();
        mUndoGroup->addStack(undoStack);
        mUndoGroup->setActiveStack(undoStack);
    } else {
        setCurrentFileName(QString());
    }
}

void MainWindow::aboutTiled()
{
    AboutDialog aboutDialog(this);
    aboutDialog.exec();
}

void MainWindow::retranslateUi()
{
    if (!mCurrentFileName.isEmpty()) {
        const QString fileName = QFileInfo(mCurrentFileName).fileName();
        setWindowTitle(tr("%1[*] - Tiled").arg(fileName));
    }

    mLayerMenu->setTitle(tr("&Layer"));
    mActionHandler->retranslateUi();
}

void MainWindow::setupQuickStamps()
{
    QList<int> keys;
    keys << Qt::Key_1
         << Qt::Key_2
         << Qt::Key_3
         << Qt::Key_4
         << Qt::Key_5
         << Qt::Key_6
         << Qt::Key_7
         << Qt::Key_8
         << Qt::Key_9;

    QSignalMapper *selectMapper = new QSignalMapper(this);
    QSignalMapper *saveMapper = new QSignalMapper(this);

    mQuickStamps.resize(keys.size());

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

    connect(selectMapper, SIGNAL(mapped(int)), SLOT(selectQuickStamp(int)));
    connect(saveMapper, SIGNAL(mapped(int)), SLOT(saveQuickStamp(int)));
}

void MainWindow::cleanQuickStamps()
{
    for (int i = 0; i < mQuickStamps.size(); i++)
        eraseQuickStamp(i);
}

void MainWindow::eraseQuickStamp(int index)
{
    if (Map *quickStamp = mQuickStamps.at(index)) {
        // Decrease reference to tilesets
        TilesetManager *tilesetManager = TilesetManager::instance();
        foreach (Tileset *tileset, quickStamp->tilesets())
            tilesetManager->removeReference(tileset);
        delete quickStamp;
    }
}

void MainWindow::selectQuickStamp(int index)
{
    if (!mMapDocument)
        return;

    if (Map *stampMap = mQuickStamps.at(index)) {
        mMapDocument->unifyTilesets(stampMap);
        setStampBrush(stampMap->layerAt(0)->asTileLayer());
        ToolManager::instance()->selectTool(mStampBrush);
    }
}

void MainWindow::saveQuickStamp(int index)
{
    const Map *map = mMapDocument->map();

    // The source of the saved stamp depends on which tool is selected
    AbstractTool *selectedTool = ToolManager::instance()->selectedTool();
    TileLayer *copy = 0;
    if (selectedTool == mStampBrush) {
        TileLayer *stamp = mStampBrush->stamp();
        if (!stamp)
            return;

        copy = static_cast<TileLayer*>(stamp->clone());
    } else {
        int currentLayer = mMapDocument->currentLayer();
        if (currentLayer == -1)
            return;

        const TileLayer *tileLayer =
                map->layerAt(currentLayer)->asTileLayer();
        if (!tileLayer)
            return;

        const QRegion &selection = mMapDocument->tileSelection();
        if (selection.isEmpty())
            return;

        copy = tileLayer->copy(selection.translated(-tileLayer->x(),
                                                    -tileLayer->y()));
    }

    Map *copyMap = new Map(map->orientation(),
                           copy->width(), copy->height(),
                           map->tileWidth(), map->tileHeight());

    copyMap->addLayer(copy);

    // Add tileset references to map and tileset manager
    TilesetManager *tilesetManager = TilesetManager::instance();
    foreach (Tileset *tileset, copy->usedTilesets()) {
        copyMap->addTileset(tileset);
        tilesetManager->addReference(tileset);
    }

    eraseQuickStamp(index);
    mQuickStamps.replace(index, copyMap);
}
