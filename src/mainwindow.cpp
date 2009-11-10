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
#include "changeselection.h"
#include "eraser.h"
#include "erasetiles.h"
#include "layer.h"
#include "layerdock.h"
#include "layermodel.h"
#include "map.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "newmapdialog.h"
#include "newtilesetdialog.h"
#include "propertiesdialog.h"
#include "resizedialog.h"
#include "saveasimagedialog.h"
#include "selectiontool.h"
#include "stampbrush.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileselectionmodel.h"
#include "tilesetdock.h"
#include "tilesetmanager.h"
#include "toolmanager.h"
#include "tmxmapreader.h"
#include "tmxmapwriter.h"
#include "undodock.h"

#include <QClipboard>
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

static const char * const TMX_MIMETYPE = "text/tmx";

#if QT_VERSION >= 0x040600
/**
 * Looks up the icon with the specified \a name from the system theme and set
 * it on the \a action when found.
 */
static void setThemeIcon(QAction *action, const char *name)
{
    QIcon themeIcon = QIcon::fromTheme(QLatin1String(name));
    if (!themeIcon.isNull())
        action->setIcon(themeIcon);
}
#endif

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
    , mUi(new Ui::MainWindow)
    , mMapDocument(0)
    , mLayerDock(new LayerDock(this))
    , mTilesetDock(new TilesetDock(this))
    , mZoomLabel(new QLabel)
    , mStatusInfoLabel(new QLabel)
{
    mUi->setupUi(this);

    QIcon redoIcon(QLatin1String(":images/16x16/edit-redo.png"));
    QIcon undoIcon(QLatin1String(":images/16x16/edit-undo.png"));

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
    addDockWidget(Qt::BottomDockWidgetArea, mTilesetDock);
    addDockWidget(Qt::RightDockWidgetArea, undoDock);

    updateZoomLabel(mUi->mapView->scale());
    connect(mUi->mapView, SIGNAL(scaleChanged(qreal)),
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
    connect(mUi->actionSave, SIGNAL(triggered()), SLOT(saveFile()));
    connect(mUi->actionSaveAs, SIGNAL(triggered()), SLOT(saveFileAs()));
    connect(mUi->actionSaveAsImage, SIGNAL(triggered()), SLOT(saveAsImage()));
    connect(mUi->actionClose, SIGNAL(triggered()), SLOT(closeFile()));
    connect(mUi->actionQuit, SIGNAL(triggered()), SLOT(close()));

    connect(mUi->actionCut, SIGNAL(triggered()), SLOT(cut()));
    connect(mUi->actionCopy, SIGNAL(triggered()), SLOT(copy()));
    connect(mUi->actionPaste, SIGNAL(triggered()), SLOT(paste()));
    connect(mUi->actionSelectAll, SIGNAL(triggered()), SLOT(selectAll()));
    connect(mUi->actionSelectNone, SIGNAL(triggered()), SLOT(selectNone()));

    connect(mUi->actionZoomIn, SIGNAL(triggered()),
            mUi->mapView, SLOT(zoomIn()));
    connect(mUi->actionZoomOut, SIGNAL(triggered()),
            mUi->mapView, SLOT(zoomOut()));
    connect(mUi->actionZoomNormal, SIGNAL(triggered()),
            mUi->mapView, SLOT(resetZoom()));

    connect(mUi->actionNewTileset, SIGNAL(triggered()), SLOT(newTileset()));
    connect(mUi->actionResizeMap, SIGNAL(triggered()), SLOT(resizeMap()));
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

    QMenu *menu = new QMenu(this);
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
         mRecentFiles[i] = new QAction(this);
         menu->addAction(mRecentFiles[i]);
         mRecentFiles[i]->setVisible(false);
         connect(mRecentFiles[i], SIGNAL(triggered()),
                 this, SLOT(openRecentFile()));
    }
    menu->addSeparator();
    QIcon clearIcon(QLatin1String(":images/16x16/edit-clear.png"));
    QAction *clear = new QAction(clearIcon,
                                 QLatin1String("Clear Recent Files"),
                                 this);
    menu->addAction(clear);
    connect(clear, SIGNAL(triggered()), this, SLOT(clearRecentFiles()));
    mUi->actionRecentFiles->setMenu(menu);

    // Qt 4.6 supports requesting icons from the system theme, at least on
    // desktops where there is a system theme (ie. Linux).
#if QT_VERSION >= 0x040600
    setThemeIcon(mUi->actionNew, "document-new");
    setThemeIcon(mUi->actionOpen, "document-open");
    setThemeIcon(mUi->actionRecentFiles, "document-open-recent");
    setThemeIcon(clear, "edit-clear");
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
#endif

    mScene = new MapScene(this);
    mUi->mapView->setScene(mScene);
    mUi->mapView->centerOn(0, 0);

    mUi->actionShowGrid->setChecked(mScene->isGridVisible());
    connect(mUi->actionShowGrid, SIGNAL(toggled(bool)),
            mScene, SLOT(setGridVisible(bool)));

    connect(mTilesetDock, SIGNAL(currentTilesChanged(const TileLayer*)),
            this, SLOT(setStampBrush(const TileLayer*)));

    mStampBrush = new StampBrush(this);

    ToolManager *toolManager = ToolManager::instance();
    toolManager->registerTool(mStampBrush);
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

    QClipboard *clipboard = QApplication::clipboard();
    connect(clipboard, SIGNAL(dataChanged()), this, SLOT(updateActions()));

    updateActions();
    readSettings();
}

MainWindow::~MainWindow()
{
    writeSettings();

    setMapDocument(0);

    ToolManager::deleteInstance();
    TilesetManager::deleteInstance();

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
            mUi->mapView->setScale(scale);

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
    TmxMapWriter mapWriter;
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
                             mUi->mapView->scale(), this);
    dialog.exec();
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

    const QRegion &selection = mMapDocument->selectionModel()->selection();
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

    int currentLayer = mMapDocument->currentLayer();
    if (currentLayer == -1)
        return;

    Map *map = mMapDocument->map();
    Layer *layer = map->layerAt(currentLayer);
    TileLayer *tileLayer = dynamic_cast<TileLayer*>(layer);
    if (!tileLayer)
        return;

    const QRegion &selection = mMapDocument->selectionModel()->selection();
    if (selection.isEmpty())
        return;

    // Create a temporary map to write to the clipboard
    TileLayer *copy = tileLayer->copy(selection.translated(-tileLayer->x(),
                                                           -tileLayer->y()));
    Map *copyMap = new Map(copy->width(), copy->height(),
                           map->tileWidth(), map->tileHeight());

    // Resolve the tilesets
    QSet<Tileset*> tilesets;
    for (int y = 0; y < copy->height(); ++y) {
        for (int x = 0; x < copy->width(); ++x) {
            Tile *tile = copy->tileAt(x, y);
            if (tile)
                tilesets.insert(tile->tileset());
        }
    }
    foreach (Tileset *tileset, tilesets)
        copyMap->addTileset(tileset);

    copyMap->addLayer(copy);
    TmxMapWriter mapWriter;

    QMimeData *mimeData = new QMimeData;
    mimeData->setData(QLatin1String(TMX_MIMETYPE),
                      mapWriter.toString(copyMap).toUtf8());

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setMimeData(mimeData);

    delete copyMap;
}

void MainWindow::paste()
{
    if (!mMapDocument)
        return;

    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mimeData = clipboard->mimeData();
    const QByteArray data = mimeData->data(QLatin1String(TMX_MIMETYPE));
    if (data.isEmpty())
        return;

    QString mapData = QString::fromUtf8(data);
    TmxMapReader reader;
    Map *map = reader.fromString(mapData);
    if (!map || map->layerCount() == 0)
        return;

    TileLayer *layer = dynamic_cast<TileLayer*>(map->layerAt(0));
    if (!layer)
        return;

    // Add tilesets that are not yet part of this map
    foreach (Tileset *tileset, map->tilesets())
        if (!mMapDocument->map()->tilesets().contains(tileset))
            mMapDocument->addTileset(tileset);

    // Reset selection and paste into the stamp brush
    selectNone();
    setStampBrush(layer);
    ToolManager::instance()->selectTool(mStampBrush);

    delete map;
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
        mMapDocument->addTileset(tileset);
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
    mUi->actionRecentFiles->setEnabled(numRecentFiles > 0);
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
        selection = mMapDocument->selectionModel()->selection();

        if (currentLayer != -1) {
            Layer *layer = mMapDocument->map()->layerAt(currentLayer);
            tileLayerSelected = dynamic_cast<TileLayer*>(layer) != 0;
        }
    }

    const QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *data = clipboard->mimeData();
    const bool mapInClipboard = data->hasFormat(QLatin1String(TMX_MIMETYPE));

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
    mUi->actionZoomIn->setEnabled(mUi->mapView->canZoomIn());
    mUi->actionZoomOut->setEnabled(mUi->mapView->canZoomOut());
    mUi->actionZoomNormal->setEnabled(scale != 1);

    mZoomLabel->setText(tr("%1%").arg(scale * 100));
}

void MainWindow::selectAll()
{
    if (!mMapDocument)
        return;

    Map *map = mMapDocument->map();
    QRect all(0, 0, map->width(), map->height());
    QUndoCommand *command = new ChangeSelection(mMapDocument, all);
    mMapDocument->undoStack()->push(command);
}

void MainWindow::selectNone()
{
    if (!mMapDocument)
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
    PropertiesDialog propertiesDialog(tr("Layer"),
                                      layer->properties(),
                                      mMapDocument->undoStack(),
                                      this);
    propertiesDialog.exec();
}

/**
 * Sets the stamp brush in response to a change in the selection in the tileset
 * view.
 */
void MainWindow::setStampBrush(const TileLayer *tiles)
{
    if (tiles)
        mStampBrush->setStamp(static_cast<TileLayer*>(tiles->clone()));
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
    mSettings.setValue(QLatin1String("mapScale"), mUi->mapView->scale());
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

    // TODO: Add support for multiple map documents
    delete mMapDocument;
    mMapDocument = mapDocument;

    if (mMapDocument) {
        connect(mapDocument, SIGNAL(currentLayerChanged(int)),
                SLOT(updateActions()));
        connect(mapDocument->selectionModel(),
                SIGNAL(selectionChanged(QRegion,QRegion)),
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
