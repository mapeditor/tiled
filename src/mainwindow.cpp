/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "aboutdialog.h"
#include "addtileset.h"
#include "changeselection.h"
#include "clipboardmanager.h"
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
#include <QInputDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QSessionManager>
#include <QTextStream>
#include <QUndoGroup>
#include <QUndoStack>
#include <QUndoView>

using namespace Tiled;
using namespace Tiled::Internal;
using namespace Tiled::Utils;

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
    , mUi(new Ui::MainWindow)
    , mMapDocument(0)
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

    mUi->actionNew->setShortcut(QKeySequence::New);
    mUi->actionOpen->setShortcut(QKeySequence::Open);
    mUi->actionSave->setShortcut(QKeySequence::Save);
    mUi->actionSaveAs->setShortcut(QKeySequence::SaveAs);
    mUi->actionClose->setShortcut(QKeySequence::Close);
#if QT_VERSION >= 0x040600
    mUi->actionQuit->setShortcut(QKeySequence::Quit);
#endif
    mUi->actionCut->setShortcut(QKeySequence::Cut);
    mUi->actionCopy->setShortcut(QKeySequence::Copy);
    mUi->actionPaste->setShortcut(QKeySequence::Paste);
    mUi->actionSelectAll->setShortcut(QKeySequence::SelectAll);
    undoAction->setShortcut(QKeySequence::Undo);
    redoAction->setShortcut(QKeySequence::Redo);
    mUi->actionZoomIn->setShortcut(QKeySequence::ZoomIn);
    mUi->actionZoomOut->setShortcut(QKeySequence::ZoomOut);

    mUi->menuEdit->insertAction(mUi->actionCut, undoAction);
    mUi->menuEdit->insertAction(mUi->actionCut, redoAction);
    mUi->menuEdit->insertSeparator(mUi->actionCut);
    mUi->mainToolBar->addAction(undoAction);
    mUi->mainToolBar->addAction(redoAction);

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
    connect(mUi->actionSelectAll, SIGNAL(triggered()), SLOT(selectAll()));
    connect(mUi->actionSelectNone, SIGNAL(triggered()), SLOT(selectNone()));
    connect(mUi->actionPreferences, SIGNAL(triggered()),
            SLOT(openPreferences()));

    connect(mUi->actionZoomIn, SIGNAL(triggered()),
            mUi->mapView->zoomable(), SLOT(zoomIn()));
    connect(mUi->actionZoomOut, SIGNAL(triggered()),
            mUi->mapView->zoomable(), SLOT(zoomOut()));
    connect(mUi->actionZoomNormal, SIGNAL(triggered()),
            mUi->mapView->zoomable(), SLOT(resetZoom()));

    connect(mUi->actionNewTileset, SIGNAL(triggered()), SLOT(newTileset()));
    connect(mUi->actionResizeMap, SIGNAL(triggered()), SLOT(resizeMap()));
    connect(mUi->actionOffsetMap, SIGNAL(triggered()), SLOT(offsetMap()));
    connect(mUi->actionMapProperties, SIGNAL(triggered()),
            SLOT(editMapProperties()));

    connect(mUi->actionAddTileLayer, SIGNAL(triggered()), SLOT(addTileLayer()));
    connect(mUi->actionAddObjectLayer, SIGNAL(triggered()),
            SLOT(addObjectLayer()));
    connect(mUi->actionDuplicateLayer, SIGNAL(triggered()),
            SLOT(duplicateLayer()));
    connect(mUi->actionMoveLayerUp, SIGNAL(triggered()), SLOT(moveLayerUp()));
    connect(mUi->actionMoveLayerDown, SIGNAL(triggered()),
            SLOT(moveLayerDown()));
    connect(mUi->actionRemoveLayer, SIGNAL(triggered()), SLOT(removeLayer()));
    connect(mUi->actionLayerProperties, SIGNAL(triggered()),
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
    setThemeIcon(mUi->actionRemoveLayer, "edit-delete");
    setThemeIcon(mUi->actionMoveLayerUp, "go-up");
    setThemeIcon(mUi->actionMoveLayerDown, "go-down");
    setThemeIcon(mUi->actionLayerProperties, "document-properties");
    setThemeIcon(mUi->actionAbout, "help-about");

    mScene = new MapScene(this);
    mUi->mapView->setScene(mScene);
    mUi->mapView->centerOn(0, 0);

    mUi->actionShowGrid->setChecked(mScene->isGridVisible());
    connect(mUi->actionShowGrid, SIGNAL(toggled(bool)),
            mScene, SLOT(setGridVisible(bool)));

    mStampBrush = new StampBrush(this);
    mBucketFillTool = new BucketFillTool(this);

    connect(mTilesetDock, SIGNAL(currentTilesChanged(const TileLayer*)),
            this, SLOT(setStampBrush(const TileLayer*)));
    connect(mStampBrush, SIGNAL(currentTilesChanged(const TileLayer*)),
            this, SLOT(setStampBrush(const TileLayer*)));

    ToolManager *toolManager = ToolManager::instance();
    toolManager->registerTool(mStampBrush);
    toolManager->registerTool(mBucketFillTool);
    toolManager->registerTool(new Eraser(this));
    toolManager->registerTool(new SelectionTool(this));

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
}

MainWindow::~MainWindow()
{
    writeSettings();

    setMapDocument(0);

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

    setCurrentFileName(QString());
    updateActions();
}

bool MainWindow::openFile(const QString &fileName)
{
    if (fileName.isEmpty() || !confirmSave())
        return false;

    // Use the XML map reader to read the map (assuming it's a .tmx file)
    // TODO: Add support for input/output plugins
    TmxMapReader mapReader;
    Map *map = mapReader.read(fileName);
    if (!map) {
        QMessageBox::critical(this, tr("Error while opening map"),
                              mapReader.errorString());
        return false;
    }

    setMapDocument(new MapDocument(map));
    mUi->mapView->centerOn(0, 0);

    setCurrentFileName(fileName);
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
        mSettings.endGroup();
    }
}

void MainWindow::openFile()
{
    const QString start = fileDialogStartLocation();
    openFile(QFileDialog::getOpenFileName(this, tr("Open Map"), start,
                                          tr("Tiled map files (*.tmx)")));
}

bool MainWindow::saveFile(const QString &fileName)
{
    if (!mMapDocument)
        return false;

    Preferences *prefs = Preferences::instance();

    TmxMapWriter mapWriter;
    mapWriter.setLayerDataFormat(prefs->layerDataFormat());
    mapWriter.setDtdEnabled(prefs->dtdEnabled());

    if (!mapWriter.write(mMapDocument->map(), fileName)) {
        QMessageBox::critical(this, tr("Error while saving map"),
                              mapWriter.errorString());
        return false;
    }

    mMapDocument->undoStack()->setClean();
    setCurrentFileName(fileName);
    return true;
}

bool MainWindow::saveFile()
{
    if (!mCurrentFileName.isEmpty())
        return saveFile(mCurrentFileName);
    else
        return saveFileAs();
}

bool MainWindow::saveFileAs()
{
    const QString start = fileDialogStartLocation();
    const QString fileName =
            QFileDialog::getSaveFileName(this, QString(), start,
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
    QString filter;
    foreach (MapWriterInterface *writer, writers) {
        if (!filter.isEmpty())
            filter += QLatin1String(";;");
        filter += writer->nameFilter();
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Export As..."),
                                                    fileDialogStartLocation(),
                                                    filter);
    if (fileName.isEmpty())
        return;

    MapWriterInterface *chosenWriter = 0;

    QString suffix = QFileInfo(fileName).completeSuffix();
    if (!suffix.isEmpty()) {
        suffix.prepend(QLatin1String("*."));
        foreach (MapWriterInterface *writer, writers)
            if (writer->nameFilter().contains(suffix, Qt::CaseInsensitive))
                chosenWriter = writer;
    }

    if (!chosenWriter) {
        QMessageBox::critical(this, tr("Unknown File Format"),
                              tr("The given filename does not have any known "
                                 "file extension."));
        return;
    }

    if (!chosenWriter->write(mMapDocument->map(), fileName)) {
        QMessageBox::critical(this, tr("Error while saving map"),
                              chosenWriter->errorString());
    }
}

void MainWindow::closeFile()
{
    if (confirmSave()) {
        setMapDocument(0);
        setCurrentFileName(QString());
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
    selectNone();
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

    if (Map *map = mClipboardManager->map()) {
        // We can currently only handle maps with a single tile layer
        if (map->layerCount() == 1) {
            if (TileLayer *layer = dynamic_cast<TileLayer*>(map->layerAt(0))) {
                QList<QUndoCommand*> addTilesetCommands;

                // Add tilesets that are not yet part of this map
                foreach (Tileset *tileset, map->tilesets()) {
                    if (!mMapDocument->map()->tilesets().contains(tileset)) {
                        addTilesetCommands.append(new AddTileset(mMapDocument,
                                                                 tileset));
                    }
                }
                if (!addTilesetCommands.isEmpty()) {
                    QUndoStack *undoStack = mMapDocument->undoStack();
                    undoStack->beginMacro(tr("Add Tilesets"));
                    foreach (QUndoCommand *command, addTilesetCommands)
                        mMapDocument->undoStack()->push(command);
                    undoStack->endMacro();
                }

                // Reset selection and paste into the stamp brush
                selectNone();
                setStampBrush(layer);
                ToolManager::instance()->selectTool(mStampBrush);

                delete map;
                return;
            }
        }

        // Need to also clean up the tilesets since they didn't get an owner
        qDeleteAll(map->tilesets());
        delete map;
    }
}

void MainWindow::openPreferences()
{
    PreferencesDialog preferencesDialog(this);
    preferencesDialog.exec();
}

void MainWindow::newTileset()
{
    if (!mMapDocument)
        return;

    Map *map = mMapDocument->map();

    NewTilesetDialog newTileset(fileDialogStartLocation(), this);
    newTileset.setTileWidth(map->tileWidth());
    newTileset.setTileHeight(map->tileHeight());

    if (Tileset *tileset = newTileset.createTileset())
        mMapDocument->undoStack()->push(new AddTileset(mMapDocument, tileset));
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
                                      mMapDocument->map()->properties(),
                                      mMapDocument->undoStack(),
                                      this);
    propertiesDialog.exec();
}

void MainWindow::updateModified()
{
    setWindowModified(!mUndoGroup->isClean());
    updateActions();
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

    mUi->actionSave->setEnabled(map && !mUndoGroup->isClean());
    mUi->actionSaveAs->setEnabled(map);
    mUi->actionSaveAsImage->setEnabled(map);
    mUi->actionClose->setEnabled(map);
    mUi->actionCut->setEnabled(tileLayerSelected && !selection.isEmpty());
    mUi->actionCopy->setEnabled(tileLayerSelected && !selection.isEmpty());
    mUi->actionPaste->setEnabled(tileLayerSelected && mapInClipboard);
    mUi->actionSelectAll->setEnabled(map);
    mUi->actionSelectNone->setEnabled(!selection.isEmpty());
    mUi->actionNewTileset->setEnabled(map);
    mUi->actionResizeMap->setEnabled(map);
    mUi->actionOffsetMap->setEnabled(map);
    mUi->actionMapProperties->setEnabled(map);
    mUi->actionAddTileLayer->setEnabled(map);
    mUi->actionAddObjectLayer->setEnabled(map);

    const int layerCount = map ? map->layerCount() : 0;
    mUi->actionDuplicateLayer->setEnabled(currentLayer >= 0);
    mUi->actionMoveLayerUp->setEnabled(currentLayer >= 0 &&
                                       currentLayer < layerCount - 1);
    mUi->actionMoveLayerDown->setEnabled(currentLayer > 0);
    mUi->actionRemoveLayer->setEnabled(currentLayer >= 0);
    mUi->actionLayerProperties->setEnabled(currentLayer >= 0);
}

void MainWindow::updateZoomLabel(qreal scale)
{
    const Zoomable *zoomable = mUi->mapView->zoomable();
    mUi->actionZoomIn->setEnabled(zoomable->canZoomIn());
    mUi->actionZoomOut->setEnabled(zoomable->canZoomOut());
    mUi->actionZoomNormal->setEnabled(scale != 1);

    mZoomLabel->setText(tr("%1%").arg(scale * 100));
}

void MainWindow::selectAll()
{
    if (!mMapDocument)
        return;

    Map *map = mMapDocument->map();
    QRect all(0, 0, map->width(), map->height());
    if (mMapDocument->tileSelection() == all)
        return;

    QUndoCommand *command = new ChangeSelection(mMapDocument, all);
    mMapDocument->undoStack()->push(command);
}

void MainWindow::selectNone()
{
    if (!mMapDocument)
        return;

    if (mMapDocument->tileSelection().isEmpty())
        return;

    QUndoCommand *command = new ChangeSelection(mMapDocument, QRegion());
    mMapDocument->undoStack()->push(command);
}

/**
 * Helper function for adding a layer after having the user choose its name.
 */
void MainWindow::addLayer(MapDocument::LayerType type)
{
    if (!mMapDocument)
        return;

    QString title;
    switch (type) {
    case MapDocument::TileLayerType:
        title = tr("Add Tile Layer"); break;
    case MapDocument::ObjectLayerType:
        title = tr("Add Object Layer"); break;
    }

    bool ok;
    QString text = QInputDialog::getText(this, title,
                                         tr("Layer name:"), QLineEdit::Normal,
                                         tr("New Layer"), &ok);
    if (ok)
        mMapDocument->addLayer(type, text);
}

void MainWindow::addTileLayer()
{
    addLayer(MapDocument::TileLayerType);
}

void MainWindow::addObjectLayer()
{
    addLayer(MapDocument::ObjectLayerType);
}

void MainWindow::duplicateLayer()
{
    if (mMapDocument)
        mMapDocument->duplicateLayer();
}

void MainWindow::moveLayerUp()
{
    if (mMapDocument)
        mMapDocument->moveLayerUp(mMapDocument->currentLayer());
}

void MainWindow::moveLayerDown()
{
    if (mMapDocument)
        mMapDocument->moveLayerDown(mMapDocument->currentLayer());
}

void MainWindow::removeLayer()
{
    if (mMapDocument)
        mMapDocument->removeLayer(mMapDocument->currentLayer());
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

    mScene->setMapDocument(mapDocument);
    mLayerDock->setMapDocument(mapDocument);
    mTilesetDock->setMapDocument(mapDocument);
    mStampBrush->setMapDocument(mapDocument);
    mBucketFillTool->setMapDocument(mapDocument);

    // TODO: Add support for multiple map documents
    delete mMapDocument;
    mMapDocument = mapDocument;

    if (mMapDocument) {
        connect(mapDocument, SIGNAL(currentLayerChanged(int)),
                SLOT(updateActions()));
        connect(mapDocument, SIGNAL(tileSelectionChanged(QRegion,QRegion)),
                SLOT(updateActions()));

        QUndoStack *undoStack = mMapDocument->undoStack();
        mUndoGroup->addStack(undoStack);
        mUndoGroup->setActiveStack(undoStack);
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
}
