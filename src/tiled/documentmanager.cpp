/*
 * documentmanager.cpp
 * Copyright 2010, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2010-2016, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "documentmanager.h"

#include "abstracttool.h"
#include "adjusttileindexes.h"
#include "brokenlinks.h"
#include "filesystemwatcher.h"
#include "map.h"
#include "mapdocument.h"
#include "maprenderer.h"
#include "mapscene.h"
#include "mapview.h"
#include "tilesetmanager.h"
#include "zoomable.h"

#include <QFileInfo>
#include <QUndoGroup>

#include <QApplication>
#include <QClipboard>
#include <QDialogButtonBox>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QProcess>
#include <QScrollBar>
#include <QTabBar>
#include <QTabWidget>
#include <QUndoStack>
#include <QVBoxLayout>

using namespace Tiled;
using namespace Tiled::Internal;

/*
 * Code based on FileUtils::showInGraphicalShell from Qt Creator
 * Copyright (C) 2016 The Qt Company Ltd.
 * Used under the terms of the GNU General Public License version 3
 */
static void showInFileManager(const QString &fileName)
{
    // Mac, Windows support folder or file.
#if defined(Q_OS_WIN)
    QStringList param;
    if (!QFileInfo(fileName).isDir())
        param += QLatin1String("/select,");
    param += QDir::toNativeSeparators(fileName);
    QProcess::startDetached(QLatin1String("explorer.exe"), param);
#elif defined(Q_OS_MAC)
    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e")
               << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                                     .arg(fileName);
    QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
    scriptArgs.clear();
    scriptArgs << QLatin1String("-e")
               << QLatin1String("tell application \"Finder\" to activate");
    QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
#else
    // We cannot select a file here, because xdg-open would open the file
    // instead of the file browser...
    QProcess::startDetached(QString(QLatin1String("xdg-open \"%1\""))
                            .arg(QFileInfo(fileName).absolutePath()));
#endif
}


namespace Tiled {
namespace Internal {

class FileChangedWarning : public QWidget
{
    Q_OBJECT

public:
    FileChangedWarning(QWidget *parent = nullptr)
        : QWidget(parent)
        , mLabel(new QLabel(this))
        , mButtons(new QDialogButtonBox(QDialogButtonBox::Yes |
                                        QDialogButtonBox::No,
                                        Qt::Horizontal,
                                        this))
    {
        mLabel->setText(tr("File change detected. Discard changes and reload the map?"));

        QHBoxLayout *layout = new QHBoxLayout;
        layout->addWidget(mLabel);
        layout->addStretch(1);
        layout->addWidget(mButtons);
        setLayout(layout);

        connect(mButtons, SIGNAL(accepted()), SIGNAL(reload()));
        connect(mButtons, SIGNAL(rejected()), SIGNAL(ignore()));
    }

signals:
    void reload();
    void ignore();

private:
    QLabel *mLabel;
    QDialogButtonBox *mButtons;
};

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

} // namespace Internal
} // namespace Tiled

DocumentManager *DocumentManager::mInstance;

DocumentManager *DocumentManager::instance()
{
    if (!mInstance)
        mInstance = new DocumentManager;
    return mInstance;
}

void DocumentManager::deleteInstance()
{
    delete mInstance;
    mInstance = nullptr;
}

DocumentManager::DocumentManager(QObject *parent)
    : QObject(parent)
    , mTabWidget(new QTabWidget)
    , mUndoGroup(new QUndoGroup(this))
    , mSelectedTool(nullptr)
    , mViewWithTool(nullptr)
    , mFileSystemWatcher(new FileSystemWatcher(this))
{
    mTabWidget->setDocumentMode(true);
    mTabWidget->setTabsClosable(true);
    mTabWidget->setMovable(true);
    mTabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(mTabWidget, SIGNAL(currentChanged(int)),
            SLOT(currentIndexChanged()));
    connect(mTabWidget, SIGNAL(tabCloseRequested(int)),
            SIGNAL(documentCloseRequested(int)));
    connect(mTabWidget->tabBar(), SIGNAL(tabMoved(int,int)),
            SLOT(documentTabMoved(int,int)));
    connect(mTabWidget->tabBar(), SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(tabContextMenuRequested(QPoint)));

    connect(mFileSystemWatcher, SIGNAL(fileChanged(QString)),
            SLOT(fileChanged(QString)));

    connect(TilesetManager::instance(), &TilesetManager::tilesetChanged,
            this, &DocumentManager::tilesetChanged);
}

DocumentManager::~DocumentManager()
{
    // All documents should be closed gracefully beforehand
    Q_ASSERT(mDocuments.isEmpty());
    delete mTabWidget;
}

QWidget *DocumentManager::widget() const
{
    return mTabWidget;
}

MapDocument *DocumentManager::currentDocument() const
{
    const int index = mTabWidget->currentIndex();
    if (index == -1)
        return nullptr;

    return mDocuments.at(index);
}

MapView *DocumentManager::currentMapView() const
{
    if (QWidget *widget = mTabWidget->currentWidget())
        return static_cast<MapViewContainer*>(widget)->mapView();

    return nullptr;
}

MapScene *DocumentManager::currentMapScene() const
{
    if (MapView *mapView = currentMapView())
        return mapView->mapScene();

    return nullptr;
}

MapView *DocumentManager::viewForDocument(MapDocument *mapDocument) const
{
    const int index = mDocuments.indexOf(mapDocument);
    if (index == -1)
        return nullptr;

    return static_cast<MapViewContainer*>(mTabWidget->widget(index))->mapView();
}

int DocumentManager::findDocument(const QString &fileName) const
{
    const QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    if (canonicalFilePath.isEmpty()) // file doesn't exist
        return -1;

    for (int i = 0; i < mDocuments.size(); ++i) {
        QFileInfo fileInfo(mDocuments.at(i)->fileName());
        if (fileInfo.canonicalFilePath() == canonicalFilePath)
            return i;
    }

    return -1;
}

void DocumentManager::switchToDocument(int index)
{
    mTabWidget->setCurrentIndex(index);
}

void DocumentManager::switchToDocument(MapDocument *mapDocument)
{
    const int index = mDocuments.indexOf(mapDocument);
    if (index != -1)
        switchToDocument(index);
}

void DocumentManager::switchToLeftDocument()
{
    const int tabCount = mTabWidget->count();
    if (tabCount < 2)
        return;

    const int currentIndex = mTabWidget->currentIndex();
    switchToDocument((currentIndex > 0 ? currentIndex : tabCount) - 1);
}

void DocumentManager::switchToRightDocument()
{
    const int tabCount = mTabWidget->count();
    if (tabCount < 2)
        return;

    const int currentIndex = mTabWidget->currentIndex();
    switchToDocument((currentIndex + 1) % tabCount);
}

void DocumentManager::addDocument(MapDocument *mapDocument)
{
    Q_ASSERT(mapDocument);
    Q_ASSERT(!mDocuments.contains(mapDocument));

    mDocuments.append(mapDocument);
    mUndoGroup->addStack(mapDocument->undoStack());

    if (!mapDocument->fileName().isEmpty())
        mFileSystemWatcher->addPath(mapDocument->fileName());

    MapView *view = new MapView;
    MapScene *scene = new MapScene(view); // scene is owned by the view
    MapViewContainer *container = new MapViewContainer(view, mapDocument, mTabWidget);

    scene->setMapDocument(mapDocument);
    view->setScene(scene);

    const int documentIndex = mDocuments.size() - 1;

    mTabWidget->addTab(container, mapDocument->displayName());
    mTabWidget->setTabToolTip(documentIndex, mapDocument->fileName());
    connect(mapDocument, SIGNAL(fileNameChanged(QString,QString)),
            SLOT(fileNameChanged(QString,QString)));
    connect(mapDocument, SIGNAL(modifiedChanged()), SLOT(updateDocumentTab()));
    connect(mapDocument, SIGNAL(saved()), SLOT(documentSaved()));

    connect(container, SIGNAL(reload()), SLOT(reloadRequested()));

    switchToDocument(documentIndex);
    centerViewOn(0, 0);
}

void DocumentManager::closeCurrentDocument()
{
    const int index = mTabWidget->currentIndex();
    if (index == -1)
        return;

    closeDocumentAt(index);
}

void DocumentManager::closeDocumentAt(int index)
{
    MapDocument *mapDocument = mDocuments.at(index);
    emit documentAboutToClose(mapDocument);

    QWidget *mapViewContainer = mTabWidget->widget(index);
    mDocuments.removeAt(index);
    mTabWidget->removeTab(index);
    delete mapViewContainer;

    if (!mapDocument->fileName().isEmpty())
        mFileSystemWatcher->removePath(mapDocument->fileName());

    delete mapDocument;
}

bool DocumentManager::reloadCurrentDocument()
{
    const int index = mTabWidget->currentIndex();
    if (index == -1)
        return false;

    return reloadDocumentAt(index);
}

bool DocumentManager::reloadDocumentAt(int index)
{
    MapDocument *oldDocument = mDocuments.at(index);

    QString error;
    MapDocument *newDocument = MapDocument::load(oldDocument->fileName(),
                                                 oldDocument->readerFormat(),
                                                 &error);
    if (!newDocument) {
        emit reloadError(tr("%1:\n\n%2").arg(oldDocument->fileName(), error));
        return false;
    }

    // Remember current view state
    MapView *mapView = viewForDocument(oldDocument);
    const int layerIndex = oldDocument->currentLayerIndex();
    const qreal scale = mapView->zoomable()->scale();
    const int horizontalPosition = mapView->horizontalScrollBar()->sliderPosition();
    const int verticalPosition = mapView->verticalScrollBar()->sliderPosition();

    // Replace old tab
    addDocument(newDocument);
    closeDocumentAt(index);
    mTabWidget->tabBar()->moveTab(mDocuments.size() - 1, index);

    // Restore previous view state
    mapView = currentMapView();
    mapView->zoomable()->setScale(scale);
    mapView->horizontalScrollBar()->setSliderPosition(horizontalPosition);
    mapView->verticalScrollBar()->setSliderPosition(verticalPosition);
    if (layerIndex > 0 && layerIndex < newDocument->map()->layerCount())
        newDocument->setCurrentLayerIndex(layerIndex);

    checkTilesetColumns(newDocument);

    return true;
}

void DocumentManager::closeAllDocuments()
{
    while (!mDocuments.isEmpty())
        closeCurrentDocument();
}

void DocumentManager::currentIndexChanged()
{
    if (mViewWithTool) {
        MapScene *mapScene = mViewWithTool->mapScene();
        mapScene->disableSelectedTool();
        mViewWithTool = nullptr;
    }

    MapDocument *mapDocument = currentDocument();

    if (mapDocument)
        mUndoGroup->setActiveStack(mapDocument->undoStack());

    emit currentDocumentChanged(mapDocument);

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

void DocumentManager::setSelectedTool(AbstractTool *tool)
{
    if (mSelectedTool == tool)
        return;

    if (mSelectedTool) {
        disconnect(mSelectedTool, &AbstractTool::cursorChanged,
                   this, &DocumentManager::cursorChanged);
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
                this, &DocumentManager::cursorChanged);
    }
}

void DocumentManager::fileNameChanged(const QString &fileName,
                                      const QString &oldFileName)
{
    if (!fileName.isEmpty())
        mFileSystemWatcher->addPath(fileName);
    if (!oldFileName.isEmpty())
        mFileSystemWatcher->removePath(oldFileName);

    updateDocumentTab();
}

void DocumentManager::updateDocumentTab()
{
    MapDocument *mapDocument = static_cast<MapDocument*>(sender());
    const int index = mDocuments.indexOf(mapDocument);

    QString tabText = mapDocument->displayName();
    if (mapDocument->isModified())
        tabText.prepend(QLatin1Char('*'));

    mTabWidget->setTabText(index, tabText);
    mTabWidget->setTabToolTip(index, mapDocument->fileName());
}

void DocumentManager::documentSaved()
{
    MapDocument *document = static_cast<MapDocument*>(sender());
    const int index = mDocuments.indexOf(document);
    Q_ASSERT(index != -1);

    QWidget *widget = mTabWidget->widget(index);
    MapViewContainer *container = static_cast<MapViewContainer*>(widget);
    container->setFileChangedWarningVisible(false);
}

void DocumentManager::documentTabMoved(int from, int to)
{
    mDocuments.move(from, to);
}

void DocumentManager::tabContextMenuRequested(const QPoint &pos)
{
    QTabBar *tabBar = mTabWidget->tabBar();
    int index = tabBar->tabAt(pos);
    if (index == -1)
        return;

    QMenu menu(mTabWidget->window());

    QString fileName = mDocuments.at(index)->fileName();

    QAction *copyPath = menu.addAction(tr("Copy File Path"));
    connect(copyPath, &QAction::triggered, [fileName] {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(QDir::toNativeSeparators(fileName));
    });

    QAction *openFolder = menu.addAction(tr("Open Containing Folder..."));
    connect(openFolder, &QAction::triggered, [fileName] {
        showInFileManager(fileName);
    });

    menu.exec(tabBar->mapToGlobal(pos));
}

void DocumentManager::fileChanged(const QString &fileName)
{
    const int index = findDocument(fileName);

    // Most likely the file was removed
    if (index == -1)
        return;

    MapDocument *document = mDocuments.at(index);

    // Ignore change event when it seems to be our own save
    if (QFileInfo(fileName).lastModified() == document->lastSaved())
        return;

    // Automatically reload when there are no unsaved changes
    if (!document->isModified()) {
        reloadDocumentAt(index);
        return;
    }

    QWidget *widget = mTabWidget->widget(index);
    MapViewContainer *container = static_cast<MapViewContainer*>(widget);
    container->setFileChangedWarningVisible(true);
}

void DocumentManager::reloadRequested()
{
    int index = mTabWidget->indexOf(static_cast<MapViewContainer*>(sender()));
    Q_ASSERT(index != -1);
    reloadDocumentAt(index);
}

void DocumentManager::cursorChanged(const QCursor &cursor)
{
    if (mViewWithTool)
        mViewWithTool->viewport()->setCursor(cursor);
}

void DocumentManager::centerViewOn(qreal x, qreal y)
{
    const int index = mTabWidget->currentIndex();
    if (index == -1)
        return;

    MapView *view = currentMapView();
    MapDocument *document = mDocuments.at(index);

    view->centerOn(document->renderer()->pixelToScreenCoords(x, y));
}

static bool mayNeedColumnCountAdjustment(const Tileset &tileset)
{
    if (tileset.isCollection())
        return false;
    if (!tileset.imageLoaded())
        return false;
    if (tileset.columnCount() == tileset.expectedColumnCount())
        return false;
    if (tileset.columnCount() == 0 || tileset.expectedColumnCount() == 0)
        return false;
    if (tileset.expectedRowCount() < 2 || tileset.rowCount() < 2)
        return false;

    return true;
}

void DocumentManager::tilesetChanged(Tileset *tileset)
{
    if (!mayNeedColumnCountAdjustment(*tileset))
        return;

    SharedTileset sharedTileset = tileset->sharedPointer();

    bool anyRelevantMap = false;
    for (MapDocument *document : mDocuments) {
        if (document->map()->tilesets().contains(sharedTileset)) {
            anyRelevantMap = true;
            break;
        }
    }

    if (anyRelevantMap && askForAdjustment(*tileset)) {
        bool tilesetAdjusted = false;

        for (MapDocument *mapDocument : mDocuments) {
            Map *map = mapDocument->map();

            if (map->tilesets().contains(sharedTileset)) {
                auto command1 = new AdjustTileIndexes(mapDocument, *tileset);
                mapDocument->undoStack()->beginMacro(command1->text());
                mapDocument->undoStack()->push(command1);

                if (!tilesetAdjusted) {
                    auto command2 = new AdjustTileMetaData(mapDocument, *tileset);
                    tilesetAdjusted = true;
                    mapDocument->undoStack()->push(command2);
                }

                mapDocument->undoStack()->endMacro();
            }
        }
    }

    tileset->syncExpectedColumnsAndRows();
}

/**
 * Checks whether the number of columns in tileset image based tilesets matches
 * with the expected amount. Offers to adjust tile indexes if not.
 */
void DocumentManager::checkTilesetColumns(MapDocument *mapDocument)
{
    for (const SharedTileset &tileset : mapDocument->map()->tilesets()) {
        if (!mayNeedColumnCountAdjustment(*tileset))
            continue;

        if (askForAdjustment(*tileset)) {
            auto command1 = new AdjustTileIndexes(mapDocument, *tileset);
            auto command2 = new AdjustTileMetaData(mapDocument, *tileset);

            mapDocument->undoStack()->beginMacro(command1->text());
            mapDocument->undoStack()->push(command1);
            mapDocument->undoStack()->push(command2);
            mapDocument->undoStack()->endMacro();
        }

        tileset->syncExpectedColumnsAndRows();
    }
}

bool DocumentManager::askForAdjustment(const Tileset &tileset)
{
    int r = QMessageBox::question(mTabWidget->window(),
                                  tr("Tileset Columns Changed"),
                                  tr("The number of tile columns in the tileset '%1' appears to have changed from %2 to %3. "
                                     "Do you want to adjust tile references?")
                                  .arg(tileset.name())
                                  .arg(tileset.expectedColumnCount())
                                  .arg(tileset.columnCount()),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::Yes);

    return r == QMessageBox::Yes;
}

#include "documentmanager.moc"
