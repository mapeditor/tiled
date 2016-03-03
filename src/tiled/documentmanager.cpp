/*
 * documentmanager.cpp
 * Copyright 2010, Stefan Beller <stefanbeller@googlemail.com>
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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
#include "filesystemwatcher.h"
#include "map.h"
#include "mapdocument.h"
#include "mapeditor.h"
#include "maprenderer.h"
#include "mapview.h"
#include "movabletabwidget.h"
#include "tilesetdocument.h"
#include "tilesetmanager.h"
#include "zoomable.h"

#include <QFileInfo>
#include <QUndoGroup>

#include <QMessageBox>
#include <QScrollBar>
#include <QTabBar>
#include <QUndoStack>
#include <QVBoxLayout>
#include <QStackedLayout>

using namespace Tiled;
using namespace Tiled::Internal;


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
    , mUndoGroup(new QUndoGroup(this))
    , mFileSystemWatcher(new FileSystemWatcher(this))
{
    mWidget = new QWidget;

    mTabBar = new QTabBar(mWidget);
    mTabBar->setExpanding(false);
    mTabBar->setDocumentMode(true);
    mTabBar->setTabsClosable(true);
    mTabBar->setMovable(true);

    mMapEditor = new MapEditor(mWidget);

    QVBoxLayout *vertical = new QVBoxLayout(mWidget);
    vertical->addWidget(mTabBar);
    vertical->setMargin(0);
    vertical->setSpacing(0);

    mEditorStack = new QStackedLayout;
    mEditorStack->addWidget(mMapEditor);
    vertical->addLayout(mEditorStack);

    connect(mTabBar, &QTabBar::currentChanged,
            this, &DocumentManager::currentIndexChanged);
    connect(mTabBar, &QTabBar::tabCloseRequested,
            this, &DocumentManager::documentCloseRequested);
    connect(mTabBar, &QTabBar::tabMoved,
            this, &DocumentManager::documentTabMoved);

    connect(mFileSystemWatcher, &FileSystemWatcher::fileChanged,
            this, &DocumentManager::fileChanged);

    connect(TilesetManager::instance(), &TilesetManager::tilesetChanged,
            this, &DocumentManager::tilesetChanged);
}

DocumentManager::~DocumentManager()
{
    // All documents should be closed gracefully beforehand
    Q_ASSERT(mDocuments.isEmpty());
    Q_ASSERT(mTilesetDocuments.isEmpty());
    delete mWidget;
}

QWidget *DocumentManager::widget() const
{
    return mWidget;
}

Document *DocumentManager::currentDocument() const
{
    const int index = mTabBar->currentIndex();
    if (index == -1)
        return nullptr;

    return mDocuments.at(index);
}

MapView *DocumentManager::currentMapView() const
{
    return mMapEditor->currentMapView();
}

MapScene *DocumentManager::currentMapScene() const
{
    // todo:
    // * get index from mTabBar
    // * get document by index
    // * ask the right main window for the map scene

//    if (MapView *mapView = currentMapView())
//        return mapView->mapScene();

    return nullptr;
}

/**
 * Returns the map view that displays the given document, or null when there
 * is none.
 */
MapView *DocumentManager::viewForDocument(MapDocument *mapDocument) const
{
    return mMapEditor->viewForDocument(mapDocument);
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
    mTabBar->setCurrentIndex(index);
}

void DocumentManager::switchToDocument(Document *document)
{
    const int index = mDocuments.indexOf(document);
    if (index != -1)
        switchToDocument(index);
}

void DocumentManager::switchToLeftDocument()
{
    const int tabCount = mTabBar->count();
    if (tabCount < 2)
        return;

    const int currentIndex = mTabBar->currentIndex();
    switchToDocument((currentIndex > 0 ? currentIndex : tabCount) - 1);
}

void DocumentManager::switchToRightDocument()
{
    const int tabCount = mTabBar->count();
    if (tabCount < 2)
        return;

    const int currentIndex = mTabBar->currentIndex();
    switchToDocument((currentIndex + 1) % tabCount);
}

void DocumentManager::addDocument(Document *document)
{
    Q_ASSERT(document);
    Q_ASSERT(!mDocuments.contains(document));

    mDocuments.append(document);
    mUndoGroup->addStack(document->undoStack());

    if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document))
        for (const SharedTileset &tileset : mapDocument->map()->tilesets())
            addToTilesetDocument(tileset, mapDocument);

    if (!document->fileName().isEmpty())
        mFileSystemWatcher->addPath(document->fileName());


    if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document))
        mMapEditor->addMapDocument(mapDocument);
    // todo: Handle TilesetDocument


    const int documentIndex = mDocuments.size() - 1;

    mTabBar->addTab(document->displayName());
    mTabBar->setTabToolTip(documentIndex, document->fileName());

    connect(document, SIGNAL(fileNameChanged(QString,QString)),
            SLOT(fileNameChanged(QString,QString)));
    connect(document, SIGNAL(modifiedChanged()), SLOT(updateDocumentTab()));
    connect(document, SIGNAL(saved()), SLOT(documentSaved()));

    if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document)) {
        connect(mapDocument, &MapDocument::tilesetAdded, this, &DocumentManager::tilesetAdded);
        connect(mapDocument, &MapDocument::tilesetRemoved, this, &DocumentManager::tilesetRemoved);
        connect(mapDocument, &MapDocument::tilesetReplaced, this, &DocumentManager::tilesetReplaced);
    }

//    connect(container, SIGNAL(reload()), SLOT(reloadRequested()));

    switchToDocument(documentIndex);
    centerViewOn(0, 0);
}

void DocumentManager::closeCurrentDocument()
{
    const int index = mTabBar->currentIndex();
    if (index == -1)
        return;

    closeDocumentAt(index);
}

void DocumentManager::closeDocumentAt(int index)
{
    Document *document = mDocuments.at(index);
    emit documentAboutToClose(document);

    mDocuments.removeAt(index);
    mTabBar->removeTab(index);

    if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document))
        mMapEditor->removeMapDocument(mapDocument);

    if (!document->fileName().isEmpty())
        mFileSystemWatcher->removePath(document->fileName());

    if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document))
        for (const SharedTileset &tileset : mapDocument->map()->tilesets())
            removeFromTilesetDocument(tileset, mapDocument);

    delete document;
}

bool DocumentManager::reloadCurrentDocument()
{
    const int index = mTabBar->currentIndex();
    if (index == -1)
        return false;

    return reloadDocumentAt(index);
}

bool DocumentManager::reloadDocumentAt(int index)
{
    return false;

    // todo: look into how to split this functionality between DocumentManager
    // and MapEditHost / TilesetEditHost

    /*
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
    mTabWidget->moveTab(mDocuments.size() - 1, index);

    // Restore previous view state
    mapView = currentMapView();
    mapView->zoomable()->setScale(scale);
    mapView->horizontalScrollBar()->setSliderPosition(horizontalPosition);
    mapView->verticalScrollBar()->setSliderPosition(verticalPosition);
    if (layerIndex > 0 && layerIndex < newDocument->map()->layerCount())
        newDocument->setCurrentLayerIndex(layerIndex);

    checkTilesetColumns(newDocument);

    return true;
    */
}

void DocumentManager::closeAllDocuments()
{
    while (!mDocuments.isEmpty())
        closeCurrentDocument();
}

void DocumentManager::currentIndexChanged()
{
    Document *document = currentDocument();

    if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document)) {
        mMapEditor->setCurrentMapDocument(mapDocument);
        mEditorStack->setCurrentWidget(mMapEditor);
    }

    if (!document)
        mMapEditor->setCurrentMapDocument(nullptr);

    if (document)
        mUndoGroup->setActiveStack(document->undoStack());

    emit currentDocumentChanged(document);
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
    Document *document = static_cast<Document*>(sender());
    const int index = mDocuments.indexOf(document);

    QString tabText = document->displayName();
    if (document->isModified())
        tabText.prepend(QLatin1Char('*'));

    mTabBar->setTabText(index, tabText);
    mTabBar->setTabToolTip(index, document->fileName());
}

void DocumentManager::documentSaved()
{
    Document *document = static_cast<Document*>(sender());
    const int index = mDocuments.indexOf(document);
    Q_ASSERT(index != -1);

    // todo:
    // * find the edit host
    // * have it hide any file changed warnings
    // or better:
    // * find a nice place for the warning in the document manager

    //QWidget *widget = mTabWidget->widget(index);
    //MapViewContainer *container = static_cast<MapViewContainer*>(widget);
    //container->setFileChangedWarningVisible(false);
}

void DocumentManager::documentTabMoved(int from, int to)
{
    mDocuments.move(from, to);
}

void DocumentManager::tilesetAdded(int index, Tileset *tileset)
{
    Q_UNUSED(index)
    MapDocument *mapDocument = static_cast<MapDocument*>(QObject::sender());
    addToTilesetDocument(tileset->sharedPointer(), mapDocument);
}

void DocumentManager::tilesetRemoved(Tileset *tileset)
{
    MapDocument *mapDocument = static_cast<MapDocument*>(QObject::sender());
    removeFromTilesetDocument(tileset->sharedPointer(), mapDocument);
}

void DocumentManager::tilesetReplaced(int index, Tileset *tileset, Tileset *oldTileset)
{
    Q_UNUSED(index)
    MapDocument *mapDocument = static_cast<MapDocument*>(QObject::sender());
    addToTilesetDocument(tileset->sharedPointer(), mapDocument);
    removeFromTilesetDocument(oldTileset->sharedPointer(), mapDocument);
}

void DocumentManager::fileChanged(const QString &fileName)
{
    const int index = findDocument(fileName);

    // Most likely the file was removed
    if (index == -1)
        return;

    Document *document = mDocuments.at(index);

    // Ignore change event when it seems to be our own save
    if (QFileInfo(fileName).lastModified() == document->lastSaved())
        return;

    // Automatically reload when there are no unsaved changes
    if (!document->isModified()) {
        reloadDocumentAt(index);
        return;
    }

    // todo: find a better place for the file changed warning

//    QWidget *widget = mTabWidget->widget(index);
//    MapViewContainer *container = static_cast<MapViewContainer*>(widget);
//    container->setFileChangedWarningVisible(true);
}

void DocumentManager::reloadRequested()
{
    // todo: verify that this can only trigger for the current document, and
    // then just reload the one.
//    int index = mTabWidget->indexOf(static_cast<MapViewContainer*>(sender()));
//    Q_ASSERT(index != -1);
//    reloadDocumentAt(index);
}

void DocumentManager::centerViewOn(qreal x, qreal y)
{
    const int index = mTabBar->currentIndex();
    if (index == -1)
        return;

    if (MapView *view = currentMapView()) {
        MapDocument *document = static_cast<MapDocument*>(mDocuments.at(index));
        view->centerOn(document->renderer()->pixelToScreenCoords(x, y));
    }
}

TilesetDocument *DocumentManager::findTilesetDocument(const SharedTileset &tileset)
{
    return mTilesetDocuments.value(tileset);
}

void DocumentManager::addToTilesetDocument(const SharedTileset &tileset, MapDocument *mapDocument)
{
    auto tilesetDocument = findTilesetDocument(tileset);

    // Create TilesetDocument instance when it doesn't exist yet
    if (!tilesetDocument) {
        tilesetDocument = new TilesetDocument(tileset);
        mTilesetDocuments.insert(tileset, tilesetDocument);
    }

    tilesetDocument->addMapDocument(mapDocument);
}

void DocumentManager::removeFromTilesetDocument(const SharedTileset &tileset, MapDocument *mapDocument)
{
    TilesetDocument *tilesetDocument = findTilesetDocument(tileset);
    Q_ASSERT(tilesetDocument);

    tilesetDocument->removeMapDocument(mapDocument);

    // Delete the TilesetDocument instance when its tileset is no longer reachable
    //
    // TODO: since the tileset document is also deleted here, the user will
    // need to be prompted to save any changes to this tileset. This affects:
    //  * Closing a map
    //  * Removing a tileset from a map
    //  * Replacing a tileset of a map
    //
    if (tilesetDocument->mapDocuments().isEmpty()) {
        mTilesetDocuments.remove(tileset);
        delete tilesetDocument;
    }
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

    return true;
}

void DocumentManager::tilesetChanged(Tileset *tileset)
{
    if (!mayNeedColumnCountAdjustment(*tileset))
        return;

    SharedTileset sharedTileset = tileset->sharedPointer();

    bool anyRelevantMap = false;
    for (Document *document : mDocuments) {
        if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document)) {
            if (mapDocument->map()->tilesets().contains(sharedTileset)) {
                anyRelevantMap = true;
                break;
            }
        }
    }

    if (anyRelevantMap && askForAdjustment(*tileset)) {
        for (Document *document : mDocuments) {
            if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document)) {
                Map *map = mapDocument->map();

                if (map->tilesets().contains(sharedTileset)) {
                    auto command = new AdjustTileIndexes(mapDocument, tileset);
                    mapDocument->undoStack()->push(command);
                }
            }
        }
    }

    tileset->syncExpectedColumnCount();
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
            auto command = new AdjustTileIndexes(mapDocument, tileset.data());
            mapDocument->undoStack()->push(command);
        }

        tileset.data()->syncExpectedColumnCount();
    }
}

bool DocumentManager::askForAdjustment(const Tileset &tileset)
{
    int r = QMessageBox::question(mWidget->window(),
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
