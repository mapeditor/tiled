/*
 * mainwindow.cpp
 * Copyright 2008-2015, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2009-2010, Jeff Bland <jksb@member.fsf.org>
 * Copyright 2009, Dennis Honeyman <arcticuno@gmail.com>
 * Copyright 2009, Christian Henz <chrhenz@gmx.de>
 * Copyright 2010, Andrew G. Crowell <overkill9999@gmail.com>
 * Copyright 2010-2011, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2016, Mamed Ibrahimov <ibramlab@gmail.com>
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
#include "actionmanager.h"
#include "addremovetileset.h"
#include "automappingmanager.h"
#include "commandbutton.h"
#include "commandmanager.h"
#include "consoledock.h"
#include "documentmanager.h"
#include "exportasimagedialog.h"
#include "languagemanager.h"
#include "layer.h"
#include "mapdocumentactionhandler.h"
#include "mapdocument.h"
#include "mapeditor.h"
#include "mapformat.h"
#include "map.h"
#include "mapobject.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "mapview.h"
#include "minimaprenderer.h"
#include "newmapdialog.h"
#include "newtilesetdialog.h"
#include "objectgroup.h"
#include "objecttypeseditor.h"
#include "offsetmapdialog.h"
#include "patreondialog.h"
#include "pluginmanager.h"
#include "resizedialog.h"
#include "templatemanager.h"
#include "terrain.h"
#include "tile.h"
#include "tilelayer.h"
#include "tilesetdocument.h"
#include "tileseteditor.h"
#include "tileset.h"
#include "tilesetmanager.h"
#include "tmxmapformat.h"
#include "undodock.h"
#include "utils.h"
#include "zoomable.h"

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
#include <QToolBar>
#include <QToolButton>
#include <QUndoGroup>
#include <QUndoStack>
#include <QUndoView>

#ifdef Q_OS_WIN
#include <QtPlatformHeaders\QWindowsWindowFunctions>
#endif

using namespace Tiled;
using namespace Tiled::Internal;
using namespace Tiled::Utils;


namespace {

template <typename Format>
struct ExportDetails
{
    Format *mFormat = nullptr;
    QString mFileName;

    ExportDetails() {}
    ExportDetails(Format *format, const QString& fileName)
        : mFormat(format)
        , mFileName(fileName)
    {}

    bool isValid() const { return mFormat != nullptr; }
};

template <typename Format>
ExportDetails<Format> chooseExportDetails(const QString &fileName,
                                          const QString &lastExportName,
                                          const QString &lastExportFilter,
                                          QWidget* window)
{
    FormatHelper<Format> helper(FileFormat::Write, MainWindow::tr("All Files (*)"));

    Preferences *pref = Preferences::instance();

    QString selectedFilter = lastExportFilter;
    QString suggestedFilename = lastExportName;

    if (suggestedFilename.isEmpty()) {
        QFileInfo baseNameInfo = QFileInfo(fileName);
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
    QString exportToFileName = QFileDialog::getSaveFileName(window, MainWindow::tr("Export As..."),
                                                    suggestedFilename,
                                                    helper.filter(),
                                                    &selectedFilter,
                                                    QFileDialog::DontConfirmOverwrite);
    if (exportToFileName.isEmpty())
        return ExportDetails<Format>();

    // If a specific filter was selected, use that format
    Format *chosenFormat = helper.formatByNameFilter(selectedFilter);

    // If not, try to find the file extension among the name filters
    QString suffix = QFileInfo(exportToFileName).completeSuffix();
    if (!chosenFormat && !suffix.isEmpty()) {
        suffix.prepend(QLatin1String("*."));

        for (Format *format : helper.formats()) {
            if (format->nameFilter().contains(suffix, Qt::CaseInsensitive)) {
                if (chosenFormat) {
                    QMessageBox::warning(window, MainWindow::tr("Non-unique file extension"),
                                         MainWindow::tr("Non-unique file extension.\n"
                                                    "Please select specific format."));
                    return chooseExportDetails<Format>(exportToFileName, lastExportName, lastExportFilter, window);
                } else {
                    chosenFormat = format;
                }
            }
        }
    }

    if (!chosenFormat) {
        QMessageBox::critical(window, MainWindow::tr("Unknown File Format"),
                              MainWindow::tr("The given filename does not have any known "
                                         "file extension."));
        return ExportDetails<Format>();
    }

    return ExportDetails<Format>(chosenFormat, exportToFileName);
}

} // namespace


MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , mActionManager(new ActionManager)
    , mUi(new Ui::MainWindow)
    , mActionHandler(new MapDocumentActionHandler(this))
    , mConsoleDock(new ConsoleDock(this))
    , mObjectTypesEditor(new ObjectTypesEditor(this))
    , mAutomappingManager(new AutomappingManager(this))
    , mDocumentManager(DocumentManager::instance())
{
    mUi->setupUi(this);

    ActionManager::registerAction(mUi->actionNewMap, "file.new_map");
    ActionManager::registerAction(mUi->actionNewTileset, "file.new_tileset");

    auto *mapEditor = new MapEditor;
    auto *tilesetEditor = new TilesetEditor;

    connect(mapEditor, &Editor::enabledStandardActionsChanged, this, &MainWindow::updateActions);
    connect(tilesetEditor, &Editor::enabledStandardActionsChanged, this, &MainWindow::updateActions);

    mDocumentManager->setEditor(Document::MapDocumentType, mapEditor);
    mDocumentManager->setEditor(Document::TilesetDocumentType, tilesetEditor);

    setCentralWidget(mDocumentManager->widget());

#ifdef Q_OS_MAC
    MacSupport::addFullscreen(this);
#endif

    setDockOptions(dockOptions() | QMainWindow::GroupedDragging);

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

    addDockWidget(Qt::BottomDockWidgetArea, mConsoleDock);

    mConsoleDock->setVisible(false);

    mUi->actionNewMap->setShortcuts(QKeySequence::New);
    mUi->actionOpen->setShortcuts(QKeySequence::Open);
    mUi->actionSave->setShortcuts(QKeySequence::Save);
    mUi->actionSaveAs->setShortcuts(QKeySequence::SaveAs);
    mUi->actionClose->setShortcuts(QKeySequence::Close);
    mUi->actionQuit->setShortcuts(QKeySequence::Quit);
    mUi->actionCut->setShortcuts(QKeySequence::Cut);
    mUi->actionCopy->setShortcuts(QKeySequence::Copy);
    mUi->actionPaste->setShortcuts(QKeySequence::Paste);
    QList<QKeySequence> deleteKeys = QKeySequence::keyBindings(QKeySequence::Delete);
    deleteKeys.removeAll(Qt::Key_D | Qt::ControlModifier);  // used as "duplicate" shortcut
#ifdef Q_OS_OSX
    // Add the Backspace key as primary shortcut for Delete, which seems to be
    // the expected one for OS X.
    if (!deleteKeys.contains(QKeySequence(Qt::Key_Backspace)))
        deleteKeys.prepend(QKeySequence(Qt::Key_Backspace));
#endif
    mUi->actionDelete->setShortcuts(deleteKeys);

    QList<QKeySequence> redoShortcuts = QKeySequence::keyBindings(QKeySequence::Redo);
    const QKeySequence ctrlY(Qt::Key_Y | Qt::ControlModifier);
    if (!redoShortcuts.contains(ctrlY))
        redoShortcuts.append(ctrlY);

    undoAction->setShortcuts(QKeySequence::Undo);
    redoAction->setShortcuts(redoShortcuts);

    auto snappingGroup = new QActionGroup(this);
    mUi->actionSnapNothing->setActionGroup(snappingGroup);
    mUi->actionSnapToGrid->setActionGroup(snappingGroup);
    mUi->actionSnapToFineGrid->setActionGroup(snappingGroup);
    mUi->actionSnapToPixels->setActionGroup(snappingGroup);

    mUi->actionShowGrid->setChecked(preferences->showGrid());
    mUi->actionShowTileObjectOutlines->setChecked(preferences->showTileObjectOutlines());
    mUi->actionShowTileAnimations->setChecked(preferences->showTileAnimations());
    mUi->actionSnapToGrid->setChecked(preferences->snapToGrid());
    mUi->actionSnapToFineGrid->setChecked(preferences->snapToFineGrid());
    mUi->actionSnapToPixels->setChecked(preferences->snapToPixels());
    mUi->actionHighlightCurrentLayer->setChecked(preferences->highlightCurrentLayer());
    mUi->actionAutoMapWhileDrawing->setChecked(preferences->automappingDrawing());

#ifdef Q_OS_MAC
    mUi->actionFullScreen->setShortcuts(QKeySequence::FullScreen);
#endif

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
            this, SLOT(reloadTilesetImages()));

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
                                mActionHandler->actionSelectInverse());
    mUi->menuEdit->insertAction(mUi->actionPreferences,
                                mActionHandler->actionSelectNone());
    mUi->menuEdit->insertSeparator(mUi->actionPreferences);

    mUi->menuMap->insertAction(mUi->actionOffsetMap,
                               mActionHandler->actionCropToSelection());

    mUi->menuMap->insertAction(mUi->actionOffsetMap,
                               mActionHandler->actionAutocrop());

    mLayerMenu = new QMenu(tr("&Layer"), this);
    mNewLayerMenu = mActionHandler->createNewLayerMenu(mLayerMenu);
    mGroupLayerMenu = mActionHandler->createGroupLayerMenu(mLayerMenu);
    mLayerMenu->addMenu(mNewLayerMenu);
    mLayerMenu->addMenu(mGroupLayerMenu);
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
    connect(mUi->actionClearRecentFiles, &QAction::triggered,
            preferences, &Preferences::clearRecentFiles);
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

    connect(mUi->actionCut, &QAction::triggered, this, &MainWindow::cut);
    connect(mUi->actionCopy, &QAction::triggered, this, &MainWindow::copy);
    connect(mUi->actionPaste, &QAction::triggered, this, &MainWindow::paste);
    connect(mUi->actionPasteInPlace, &QAction::triggered, this, &MainWindow::pasteInPlace);
    connect(mUi->actionDelete, &QAction::triggered, this, &MainWindow::delete_);
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
    connect(mUi->actionSnapToPixels, SIGNAL(toggled(bool)),
            preferences, SLOT(setSnapToPixels(bool)));
    connect(mUi->actionHighlightCurrentLayer, SIGNAL(toggled(bool)),
            preferences, SLOT(setHighlightCurrentLayer(bool)));
    connect(mUi->actionZoomIn, SIGNAL(triggered()), SLOT(zoomIn()));
    connect(mUi->actionZoomOut, SIGNAL(triggered()), SLOT(zoomOut()));
    connect(mUi->actionZoomNormal, SIGNAL(triggered()), SLOT(zoomNormal()));
    connect(mUi->actionFullScreen, &QAction::toggled, this, &MainWindow::setFullScreen);
    connect(mUi->actionClearView, &QAction::toggled, this, &MainWindow::toggleClearView);

    CommandManager::instance()->registerMenu(mUi->menuCommand);

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

    connect(mUi->actionTilesetProperties, &QAction::triggered,
            this, &MainWindow::editTilesetProperties);

    connect(mUi->actionDocumentation, SIGNAL(triggered()), SLOT(openDocumentation()));
    connect(mUi->actionBecomePatron, SIGNAL(triggered()), SLOT(becomePatron()));
    connect(mUi->actionAbout, SIGNAL(triggered()), SLOT(aboutTiled()));

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
    setThemeIcon(mUi->actionResizeMap, "document-page-setup");
    setThemeIcon(mUi->actionMapProperties, "document-properties");
    setThemeIcon(mUi->actionDocumentation, "help-contents");
    setThemeIcon(mUi->actionAbout, "help-about");


    // Add the 'Views and Toolbars' submenu. This needs to happen after all
    // the dock widgets and toolbars have been added to the main window.
    mViewsAndToolbarsMenu = new QMenu(this);
    mViewsAndToolbarsAction = new QAction(tr("Views and Toolbars"), this);
    mViewsAndToolbarsAction->setMenu(mViewsAndToolbarsMenu);

    mResetToDefaultLayout = new QAction(tr("Reset to Default Layout"), this);

    mShowObjectTypesEditor = new QAction(tr("Object Types Editor"), this);
    mShowObjectTypesEditor->setCheckable(true);
    mUi->menuView->insertAction(mUi->actionShowGrid, mViewsAndToolbarsAction);
    mUi->menuView->insertAction(mUi->actionShowGrid, mShowObjectTypesEditor);
    mUi->menuView->insertSeparator(mUi->actionShowGrid);

    mUi->menuTileset->insertAction(mUi->actionTilesetProperties, tilesetEditor->showAnimationEditor());
    mUi->menuTileset->insertAction(mUi->actionTilesetProperties, tilesetEditor->editCollisionAction());
    mUi->menuTileset->insertAction(mUi->actionTilesetProperties, tilesetEditor->editTerrainAction());
    mUi->menuTileset->insertSeparator(mUi->actionTilesetProperties);
    mUi->menuTileset->insertAction(mUi->actionTilesetProperties, tilesetEditor->addTilesAction());
    mUi->menuTileset->insertAction(mUi->actionTilesetProperties, tilesetEditor->removeTilesAction());
    mUi->menuTileset->insertSeparator(mUi->actionTilesetProperties);

    connect(mViewsAndToolbarsMenu, &QMenu::aboutToShow,
            this, &MainWindow::updateViewsAndToolbarsMenu);

    connect(mShowObjectTypesEditor, SIGNAL(toggled(bool)),
            mObjectTypesEditor, SLOT(setVisible(bool)));
    connect(mObjectTypesEditor, SIGNAL(closed()), SLOT(onObjectTypesEditorClosed()));

    connect(ClipboardManager::instance(), SIGNAL(hasMapChanged()), SLOT(updateActions()));

    connect(mDocumentManager, SIGNAL(fileOpenRequested(QString)),
            this, SLOT(openFile(QString)));
    connect(mDocumentManager, SIGNAL(fileOpenRequested()),
            this, SLOT(openFile()));
    connect(mDocumentManager, SIGNAL(fileSaveRequested()),
            this, SLOT(saveFile()));
    connect(mDocumentManager, &DocumentManager::currentDocumentChanged,
            this, &MainWindow::documentChanged);
    connect(mDocumentManager, SIGNAL(documentCloseRequested(int)),
            this, SLOT(closeDocument(int)));
    connect(mDocumentManager, SIGNAL(reloadError(QString)),
            this, SLOT(reloadError(QString)));

    connect(mResetToDefaultLayout, &QAction::triggered, this, &MainWindow::resetToDefaultLayout);

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

    connect(qApp, &QApplication::commitDataRequest, this, &MainWindow::commitData);

    QShortcut *copyPositionShortcut = new QShortcut(tr("Alt+C"), this);
    connect(copyPositionShortcut, SIGNAL(activated()),
            mActionHandler, SLOT(copyPosition()));

    updateActions();
    updateZoomActions();
    readSettings();

    connect(mAutomappingManager, SIGNAL(warningsOccurred(bool)),
            this, SLOT(autoMappingWarning(bool)));
    connect(mAutomappingManager, SIGNAL(errorsOccurred(bool)),
            this, SLOT(autoMappingError(bool)));

#ifdef Q_OS_WIN
    connect(preferences, &Preferences::useOpenGLChanged, this, &MainWindow::ensureHasBorderInFullScreen);
#endif

    connect(preferences, &Preferences::recentFilesChanged, this, &MainWindow::updateRecentFilesMenu);

    QTimer::singleShot(500, this, [this,preferences]() {
        if (preferences->shouldShowPatreonDialog())
            becomePatron();
    });
}

MainWindow::~MainWindow()
{
    mDocumentManager->closeAllDocuments();

    // This needs to happen before deleting the TilesetManager, otherwise
    // tileset references may remain. It also needs to be done before deleting
    // the Preferences.
    mDocumentManager->deleteEditor(Document::MapDocumentType);
    mDocumentManager->deleteEditor(Document::TilesetDocumentType);

    DocumentManager::deleteInstance();
    TemplateManager::deleteInstance();
    TilesetManager::deleteInstance();
    Preferences::deleteInstance();
    LanguageManager::deleteInstance();
    PluginManager::deleteInstance();
    ClipboardManager::deleteInstance();
    CommandManager::deleteInstance();

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

bool MainWindow::event(QEvent *event)
{
#ifdef Q_OS_WIN
    if (event->type() == QEvent::WinIdChange)
        ensureHasBorderInFullScreen();
#endif

    return QMainWindow::event(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (confirmAllSave()) {
        // Make sure user won't end up in Clear View mode on next launch
        toggleClearView(false);
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    QMainWindow::changeEvent(event);
    switch (event->type()) {
    case QEvent::LanguageChange:
        mUi->retranslateUi(this);
        retranslateUi();
        break;
    case QEvent::WindowStateChange:
        mUi->actionFullScreen->setChecked(isFullScreen());
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
    const auto urls = e->mimeData()->urls();
    for (const QUrl &url : urls)
        openFile(url.toLocalFile());
}

void MainWindow::newMap()
{
    NewMapDialog newMapDialog(this);
    QScopedPointer<MapDocument> mapDocument(newMapDialog.createMap());

    if (!mapDocument)
        return;

    if (!mDocumentManager->saveDocumentAs(mapDocument.data()))
        return;

    mDocumentManager->addDocument(mapDocument.take());
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
        const auto formats = PluginManager::objects<FileFormat>();
        for (FileFormat *format : formats) {
            if (format->supportsFile(fileName)) {
                fileFormat = format;
                break;
            }
        }
    }

    if (!fileFormat) {
        QMessageBox::critical(this, tr("Error Opening File"), tr("Unrecognized file format."));
        return false;
    }

    QString error;
    Document *document = nullptr;

    if (MapFormat *mapFormat = qobject_cast<MapFormat*>(fileFormat)) {
        document = MapDocument::load(fileName, mapFormat, &error);
    } else if (TilesetFormat *tilesetFormat = qobject_cast<TilesetFormat*>(fileFormat)) {
        // It could be, that we have already loaded this tileset while loading some map.
        if (TilesetDocument *tilesetDocument = mDocumentManager->findTilesetDocument(fileName)) {
            document = tilesetDocument;
        } else {
            document = TilesetDocument::load(fileName, tilesetFormat, &error);
        }
    }

    if (!document) {
        QMessageBox::critical(this, tr("Error Opening File"), error);
        return false;
    }

    mDocumentManager->addDocument(document);

    if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document))
        mDocumentManager->checkTilesetColumns(mapDocument);

    Preferences::instance()->addRecentFile(fileName);
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

    auto preferences = Preferences::instance();
    const auto fileNames = QFileDialog::getOpenFileNames(this, tr("Open File"),
                                                         preferences->fileDialogStartLocation(),
                                                         helper.filter(),
                                                         &selectedFilter);
    if (fileNames.isEmpty())
        return;

    // When a particular filter was selected, use the associated format
    FileFormat *fileFormat = helper.formatByNameFilter(selectedFilter);

    mSettings.setValue(QLatin1String("lastUsedOpenFilter"), selectedFilter);
    for (const QString &fileName : fileNames)
        openFile(fileName, fileFormat);
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
        return mDocumentManager->saveDocumentAs(document);
    else
        return mDocumentManager->saveDocument(document, currentFileName);
}

bool MainWindow::saveFileAs()
{
    Document *document = mDocumentManager->currentDocument();
    if (!document)
        return false;

    document = saveAsDocument(document);

    return mDocumentManager->saveDocumentAs(document);
}

static bool isEmbeddedTilesetDocument(Document *document)
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
            if (!mDocumentManager->saveDocumentAs(document))
                return;
        } else if (!document->save(fileName, &error)) {
            mDocumentManager->switchToDocument(document);
            QMessageBox::critical(this, tr("Error Saving File"), error);
            return;
        }

        Preferences::instance()->addRecentFile(fileName);
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
        mDocumentManager->abortMultiDocumentClose();
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
    auto mapDocument = qobject_cast<MapDocument*>(mDocument);
    if (!mapDocument)
        return;

    QString exportFileName = mapDocument->lastExportFileName();

    if (!exportFileName.isEmpty()) {
        MapFormat *exportFormat = mapDocument->exportFormat();
        TmxMapFormat tmxFormat;

        if (!exportFormat)
            exportFormat = &tmxFormat;

        if (exportFormat->write(mapDocument->map(), exportFileName)) {
            auto *editor = static_cast<MapEditor*>(mDocumentManager->editor(Document::MapDocumentType));
            editor->showMessage(tr("Exported to %1").arg(exportFileName), 3000);
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
    if (auto mapDocument = qobject_cast<MapDocument*>(mDocument)) {
        exportMapAs(mapDocument);
    } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(mDocument)) {
        exportTilesetAs(tilesetDocument);
    }
}

void MainWindow::exportAsImage()
{
    auto mapDocument = qobject_cast<MapDocument*>(mDocument);
    if (!mapDocument)
        return;

    MapView *mapView = mDocumentManager->currentMapView();
    ExportAsImageDialog dialog(mapDocument,
                               mapDocument->fileName(),
                               mapView->zoomable()->scale(),
                               this);
    dialog.exec();
}

void MainWindow::reload()
{
    // todo: asking to save is not appropriate here
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
    if (auto editor = mDocumentManager->currentEditor())
        editor->performStandardAction(Editor::CutAction);
}

void MainWindow::copy()
{
    if (auto editor = mDocumentManager->currentEditor())
        editor->performStandardAction(Editor::CopyAction);
}

void MainWindow::paste()
{
    if (auto editor = mDocumentManager->currentEditor())
        editor->performStandardAction(Editor::PasteAction);
}

void MainWindow::pasteInPlace()
{
    if (auto editor = mDocumentManager->currentEditor())
        editor->performStandardAction(Editor::PasteInPlaceAction);
}

void MainWindow::delete_()
{
    if (auto editor = mDocumentManager->currentEditor())
        editor->performStandardAction(Editor::DeleteAction);
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
    if (mZoomable)
        mZoomable->zoomIn();
}

void MainWindow::zoomOut()
{
    if (mZoomable)
        mZoomable->zoomOut();
}

void MainWindow::zoomNormal()
{
    if (mZoomable)
        mZoomable->resetZoom();
}

void MainWindow::setFullScreen(bool fullScreen)
{
    if (isFullScreen() == fullScreen)
        return;

    if (fullScreen)
        setWindowState(windowState() | Qt::WindowFullScreen);
    else
        setWindowState(windowState() & ~Qt::WindowFullScreen);
}

void MainWindow::toggleClearView(bool clearView)
{
    if (clearView) {
        mMainWindowStates.insert(this, saveState());

        QList<QDockWidget*> docks = findChildren<QDockWidget*>(QString(), Qt::FindDirectChildrenOnly);
        QList<QToolBar*> toolBars = findChildren<QToolBar*>(QString(), Qt::FindDirectChildrenOnly);

        for (Editor *editor : mDocumentManager->editors()) {
            if (auto editorWindow = qobject_cast<QMainWindow*>(editor->editorWidget()))
                mMainWindowStates.insert(editorWindow, editorWindow->saveState());

            docks += editor->dockWidgets();
            toolBars += editor->toolBars();
        }

        for (auto dock : docks)
            dock->hide();
        for (auto toolBar : toolBars)
            toolBar->hide();

    } else {
        QMapIterator<QMainWindow*, QByteArray> it(mMainWindowStates);
        while (it.hasNext()) {
            it.next();
            it.key()->restoreState(it.value());
        }
        mMainWindowStates.clear();
    }
}

bool MainWindow::newTileset(const QString &path)
{
    Preferences *prefs = Preferences::instance();

    const QString startLocation = path.isEmpty()
            ? QFileInfo(prefs->lastPath(Preferences::ImageFile)).absolutePath()
            : path;

    NewTilesetDialog newTileset(this);
    newTileset.setImagePath(startLocation);

    SharedTileset tileset = newTileset.createTileset();
    if (!tileset)
        return false;

    if (tileset->imageSource().isLocalFile())
        prefs->setLastPath(Preferences::ImageFile, tileset->imageSource().toLocalFile());

    auto mapDocument = qobject_cast<MapDocument*>(mDocument);

    if (mapDocument && newTileset.isEmbedded()) {
        // Add embedded tileset to the map
        mapDocument->undoStack()->push(new AddTileset(mapDocument, tileset));
    } else {
        // Save new external tileset and open it
        QScopedPointer<TilesetDocument> tilesetDocument(new TilesetDocument(tileset));
        if (!mDocumentManager->saveDocumentAs(tilesetDocument.data()))
            return false;
        mDocumentManager->addDocument(tilesetDocument.take());
    }
    return true;
}

void MainWindow::reloadTilesetImages()
{
    TilesetManager *tilesetManager = TilesetManager::instance();

    if (auto mapDocument = qobject_cast<MapDocument*>(mDocument)) {
        Map *map = mapDocument->map();
        const auto tilesets = map->tilesets();
        for (const SharedTileset &tileset : tilesets)
            tilesetManager->reloadImages(tileset);
    } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(mDocument)) {
        tilesetManager->reloadImages(tilesetDocument->tileset());
    }
}

void MainWindow::addExternalTileset()
{
    auto mapDocument = qobject_cast<MapDocument*>(mDocument);
    if (!mapDocument)
        return;

    QString filter = tr("All Files (*)");

    QString selectedFilter = TsxTilesetFormat().nameFilter();

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

    auto *mapEditor = static_cast<MapEditor*>(mDocumentManager->currentEditor());
    mapEditor->addExternalTilesets(fileNames);
}

void MainWindow::resizeMap()
{
    auto mapDocument = qobject_cast<MapDocument*>(mDocument);
    if (!mapDocument)
        return;

    Map *map = mapDocument->map();

    QSize mapSize(map->size());
    QPoint mapStart(0, 0);

    if (map->infinite()) {
        QRect mapBounds;

        LayerIterator iterator(map);
        while (Layer *layer = iterator.next()) {
            if (TileLayer *tileLayer = dynamic_cast<TileLayer*>(layer))
                mapBounds = mapBounds.united(tileLayer->bounds());
        }

        if (!mapBounds.isEmpty()) {
            mapSize = mapBounds.size();
            mapStart = mapBounds.topLeft();
        }
    }

    ResizeDialog resizeDialog(this);
    resizeDialog.setOldSize(mapSize);

    // TODO: Look into fixing up the preview for maps that do not use square
    // tiles, and possibly also staggered maps.
    if (map->orientation() == Map::Orthogonal && map->tileWidth() == map->tileHeight()) {
        resizeDialog.setMiniMapRenderer([mapDocument](QSize size){
            QImage image(size, QImage::Format_ARGB32_Premultiplied);
            MiniMapRenderer(mapDocument->map()).renderToImage(image, MiniMapRenderer::DrawMapObjects
                                                              | MiniMapRenderer::DrawImageLayers
                                                              | MiniMapRenderer::DrawTileLayers
                                                              | MiniMapRenderer::IgnoreInvisibleLayer
                                                              | MiniMapRenderer::SmoothPixmapTransform);
            return image;
        });
    }

    if (resizeDialog.exec()) {
        const QSize &newSize = resizeDialog.newSize();
        const QPoint &offset = resizeDialog.offset() - mapStart;
        if (newSize != mapSize || !offset.isNull())
            mapDocument->resizeMap(newSize, offset, resizeDialog.removeObjects());
    }
}

void MainWindow::offsetMap()
{
    auto mapDocument = qobject_cast<MapDocument*>(mDocument);
    if (!mapDocument)
        return;

    OffsetMapDialog offsetDialog(mapDocument, this);
    if (offsetDialog.exec()) {
        const auto layers = offsetDialog.affectedLayers();
        if (layers.empty())
            return;

        mapDocument->offsetMap(layers,
                               offsetDialog.offset(),
                               offsetDialog.affectedBoundingRect(),
                               offsetDialog.wrapX(),
                               offsetDialog.wrapY());
    }
}

void MainWindow::editMapProperties()
{
    auto mapDocument = qobject_cast<MapDocument*>(mDocument);
    if (!mapDocument)
        return;

    mapDocument->setCurrentObject(mapDocument->map());
    emit mapDocument->editCurrentObject();
}

void MainWindow::editTilesetProperties()
{
    auto tilesetDocument = qobject_cast<TilesetDocument*>(mDocument);
    if (!tilesetDocument)
        return;

    tilesetDocument->setCurrentObject(tilesetDocument->tileset().data());
    emit tilesetDocument->editCurrentObject();
}

void MainWindow::autoMappingError(bool automatic)
{
    QString error = mAutomappingManager->errorString();
    if (!error.isEmpty()) {
        if (automatic) {
            auto *editor = static_cast<MapEditor*>(mDocumentManager->editor(Document::MapDocumentType));
            editor->showMessage(error, 3000);
        } else {
            QMessageBox::critical(this, tr("Automatic Mapping Error"), error);
        }
    }
}

void MainWindow::autoMappingWarning(bool automatic)
{
    QString warning = mAutomappingManager->warningString();
    if (!warning.isEmpty()) {
        if (automatic) {
            auto *editor = static_cast<MapEditor*>(mDocumentManager->editor(Document::MapDocumentType));
            editor->showMessage(warning, 3000);
        } else {
            QMessageBox::warning(this, tr("Automatic Mapping Warning"), warning);
        }
    }
}

void MainWindow::onObjectTypesEditorClosed()
{
    mShowObjectTypesEditor->setChecked(false);
}

void MainWindow::ensureHasBorderInFullScreen()
{
#ifdef Q_OS_WIN
    // Workaround issue #1576
    static bool hasBorderInFullScreen = false;

    if (hasBorderInFullScreen)
        return;

    if (!Preferences::instance()->useOpenGL())
        return;

    QWindow *window = windowHandle();
    if (!window)
        return;

    bool wasFullScreen = isFullScreen();
    setFullScreen(false);
    QWindowsWindowFunctions::setHasBorderInFullScreen(window, true);
    setFullScreen(wasFullScreen);

    hasBorderInFullScreen = true;
#endif
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        openFile(action->data().toString());
}

/**
 * Updates the recent files menu.
 */
void MainWindow::updateRecentFilesMenu()
{
    const QStringList files = Preferences::instance()->recentFiles();
    const int numRecentFiles = qMin<int>(files.size(), Preferences::MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i) {
        mRecentFiles[i]->setText(QFileInfo(files[i]).fileName());
        mRecentFiles[i]->setData(files[i]);
        mRecentFiles[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < Preferences::MaxRecentFiles; ++j) {
        mRecentFiles[j]->setVisible(false);
    }
    mUi->menuRecentFiles->setEnabled(numRecentFiles > 0);
}

void MainWindow::resetToDefaultLayout()
{
    // Make sure we're not in Clear View mode
    mUi->actionClearView->setChecked(false);

    // Reset the Console dock
    addDockWidget(Qt::BottomDockWidgetArea, mConsoleDock);
    mConsoleDock->setVisible(false);

    // Reset the layout of the current editor
    mDocumentManager->currentEditor()->resetLayout();
}

void MainWindow::updateViewsAndToolbarsMenu()
{
    mViewsAndToolbarsMenu->clear();

    mViewsAndToolbarsMenu->addAction(mConsoleDock->toggleViewAction());

    if (Editor *editor = mDocumentManager->currentEditor()) {
        mViewsAndToolbarsMenu->addSeparator();
        const auto dockWidgets = editor->dockWidgets();
        for (auto dockWidget : dockWidgets)
            mViewsAndToolbarsMenu->addAction(dockWidget->toggleViewAction());

        mViewsAndToolbarsMenu->addSeparator();
        const auto toolBars = editor->toolBars();
        for (auto toolBar : toolBars)
            mViewsAndToolbarsMenu->addAction(toolBar->toggleViewAction());

        mViewsAndToolbarsMenu->addSeparator();
        mViewsAndToolbarsMenu->addAction(mResetToDefaultLayout);
    }
}

void MainWindow::updateActions()
{
    const auto editor = mDocumentManager->currentEditor();
    const auto document = mDocumentManager->currentDocument();
    const auto mapDocument = qobject_cast<const MapDocument*>(document);
    const auto tilesetDocument = qobject_cast<const TilesetDocument*>(document);

    Editor::StandardActions standardActions;
    if (editor)
        standardActions = editor->enabledStandardActions();

    mUi->actionSave->setEnabled(document);
    mUi->actionSaveAs->setEnabled(document);
    mUi->actionSaveAll->setEnabled(document);

    mUi->actionExportAsImage->setEnabled(mapDocument);
    mUi->actionExport->setEnabled(mapDocument);
    mUi->actionExportAs->setEnabled(mapDocument || tilesetDocument);
    mUi->actionReload->setEnabled(mapDocument || (tilesetDocument && tilesetDocument->canReload()));
    mUi->actionClose->setEnabled(document);
    mUi->actionCloseAll->setEnabled(document);

    mUi->actionCut->setEnabled(standardActions & Editor::CutAction);
    mUi->actionCopy->setEnabled(standardActions & Editor::CopyAction);
    mUi->actionPaste->setEnabled(standardActions & Editor::PasteAction);
    mUi->actionPasteInPlace->setEnabled(standardActions & Editor::PasteInPlaceAction);
    mUi->actionDelete->setEnabled(standardActions & Editor::DeleteAction);

    mUi->menuMap->menuAction()->setVisible(mapDocument);
    mUi->actionAddExternalTileset->setEnabled(mapDocument);
    mUi->actionResizeMap->setEnabled(mapDocument);
    mUi->actionOffsetMap->setEnabled(mapDocument);
    mUi->actionMapProperties->setEnabled(mapDocument);
    mUi->actionAutoMap->setEnabled(mapDocument);

    mUi->menuTileset->menuAction()->setVisible(tilesetDocument);
    mUi->actionTilesetProperties->setEnabled(tilesetDocument);

    mLayerMenu->menuAction()->setVisible(mapDocument);
}

void MainWindow::updateZoomable()
{
    Zoomable *zoomable = nullptr;
    if (auto editor = mDocumentManager->currentEditor())
        zoomable = editor->zoomable();

    if (zoomable != mZoomable) {
        if (mZoomable)
            mZoomable->disconnect(this);

        mZoomable = zoomable;

        if (zoomable) {
            connect(zoomable, &Zoomable::scaleChanged, this, &MainWindow::updateZoomActions);
            connect(zoomable, &Zoomable::destroyed, this, [=] {
                if (mZoomable == zoomable)
                    mZoomable = nullptr;
            });
        }

        updateZoomActions();
    }
}

void MainWindow::updateZoomActions()
{
    const qreal scale = mZoomable ? mZoomable->scale() : 1;

    mUi->actionZoomIn->setEnabled(mZoomable && mZoomable->canZoomIn());
    mUi->actionZoomOut->setEnabled(mZoomable && mZoomable->canZoomOut());
    mUi->actionZoomNormal->setEnabled(scale != 1);
}

void MainWindow::openDocumentation()
{
    QDesktopServices::openUrl(QUrl(QLatin1String("http://doc.mapeditor.org")));
}

void MainWindow::writeSettings()
{
#ifdef Q_OS_MAC
    // See QTBUG-45241
    if (isFullScreen())
        setWindowState(windowState() & ~Qt::WindowFullScreen);
#endif

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

    mDocumentManager->saveState();
}

void MainWindow::readSettings()
{
    mSettings.beginGroup(QLatin1String("mainwindow"));
    QByteArray geom = mSettings.value(QLatin1String("geometry")).toByteArray();
    if (!geom.isEmpty())
        restoreGeometry(geom);
    else
        resize(Utils::dpiScaled(QSize(1200, 700)));
    restoreState(mSettings.value(QLatin1String("state"),
                                 QByteArray()).toByteArray());
    mSettings.endGroup();
    updateRecentFilesMenu();

    mDocumentManager->restoreState();
}

void MainWindow::updateWindowTitle()
{
    if (Document *document = mDocumentManager->currentDocument()) {
        setWindowTitle(tr("[*]%1").arg(document->displayName()));
        setWindowFilePath(document->fileName());
        setWindowModified(document->isModified());
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
    mNewLayerMenu->setTitle(tr("&New"));
    mGroupLayerMenu->setTitle(tr("&Group"));
    mViewsAndToolbarsAction->setText(tr("Views and Toolbars"));
    mActionHandler->retranslateUi();
    CommandManager::instance()->retranslateUi();
}

void MainWindow::exportMapAs(MapDocument *mapDocument)
{
    QString fileName = mapDocument->fileName();
    QString selectedFilter =
            mSettings.value(QLatin1String("lastUsedExportFilter")).toString();
    auto exportDetails = chooseExportDetails<MapFormat>(fileName,
                                                        mapDocument->lastExportFileName(),
                                                        selectedFilter,
                                                        this);
    if (!exportDetails.isValid()) {
        return;
    }

    // Check if writer will overwrite existing files here because some writers
    // could save to multiple files at the same time. For example CSV saves
    // each layer into a separate file.
    QStringList outputFiles = exportDetails.mFormat->outputFiles(mapDocument->map(), exportDetails.mFileName);
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

    Preferences *pref = Preferences::instance();

    pref->setLastPath(Preferences::ExportedFile, QFileInfo(exportDetails.mFileName).path());
    mSettings.setValue(QLatin1String("lastUsedExportFilter"), selectedFilter);

    auto exportResult = exportDetails.mFormat->write(mapDocument->map(), exportDetails.mFileName);
    if (!exportResult) {
        QMessageBox::critical(this, tr("Error Exporting Map!"),
                              exportDetails.mFormat->errorString());
    } else {
        // Remember export parameters, so subsequent exports can be done faster
        mapDocument->setLastExportFileName(exportDetails.mFileName);
        mapDocument->setExportFormat(exportDetails.mFormat);
    }
}

void MainWindow::exportTilesetAs(TilesetDocument *tilesetDocument)
{
    QString fileName = tilesetDocument->fileName();
    QString selectedFilter =
            mSettings.value(QLatin1String("lastUsedExportFilter")).toString();
    auto exportDetails = chooseExportDetails<TilesetFormat>(fileName,
                                                    tilesetDocument->lastExportFileName(),
                                                    selectedFilter,
                                                    this);
    if (!exportDetails.isValid())
        return;

    Preferences *pref = Preferences::instance();

    pref->setLastPath(Preferences::ExportedFile, QFileInfo(exportDetails.mFileName).path());
    mSettings.setValue(QLatin1String("lastUsedExportFilter"), selectedFilter);

    auto exportResult = exportDetails.mFormat->write(*tilesetDocument->tileset(), exportDetails.mFileName);
    if (!exportResult) {
        QMessageBox::critical(this, tr("Error Exporting Map!"),
                              exportDetails.mFormat->errorString());
    } else {
        // Remember export parameters, so subsequent exports can be done faster
        tilesetDocument->setLastExportFileName(exportDetails.mFileName);
        tilesetDocument->setExportFormat(exportDetails.mFormat);
    }
}

void MainWindow::documentChanged(Document *document)
{
    if (mDocument)
        mDocument->disconnect(this);

    mDocument = document;

    if (document) {
        connect(document, SIGNAL(fileNameChanged(QString,QString)),
                SLOT(updateWindowTitle()));
    }

    MapDocument *mapDocument = qobject_cast<MapDocument*>(document);

    if (mapDocument) {
        connect(mapDocument, &MapDocument::currentLayerChanged,
                this, &MainWindow::updateActions);
        connect(mapDocument, &MapDocument::selectedAreaChanged,
                this, &MainWindow::updateActions);
        connect(mapDocument, &MapDocument::selectedObjectsChanged,
                this, &MainWindow::updateActions);
    }

    mActionHandler->setMapDocument(mapDocument);
    mAutomappingManager->setMapDocument(mapDocument);

    updateWindowTitle();
    updateActions();
    updateZoomable();
}

void MainWindow::closeDocument(int index)
{
    if (confirmSave(mDocumentManager->documents().at(index)))
        mDocumentManager->closeDocumentAt(index);
}

void MainWindow::reloadError(const QString &error)
{
    QMessageBox::critical(this, tr("Error Reloading Map"), error);
}
