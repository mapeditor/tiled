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
#include "editor.h"
#include "filechangedwarning.h"
#include "filesystemwatcher.h"
#include "mapdocument.h"
#include "mapeditor.h"
#include "map.h"
#include "maprenderer.h"
#include "mapview.h"
#include "noeditorwidget.h"
#include "tilesetdocument.h"
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
#include <QStackedLayout>

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
    , mWidget(new QWidget)
    , mNoEditorWidget(new NoEditorWidget(mWidget))
    , mTabBar(new QTabBar(mWidget))
    , mFileChangedWarning(new FileChangedWarning(mWidget))
    , mMapEditor(nullptr) // todo: look into removing this
    , mUndoGroup(new QUndoGroup(this))
    , mFileSystemWatcher(new FileSystemWatcher(this))
{
    mTabBar->setExpanding(false);
    mTabBar->setDocumentMode(true);
    mTabBar->setTabsClosable(true);
    mTabBar->setMovable(true);
    mTabBar->setContextMenuPolicy(Qt::CustomContextMenu);

    mFileChangedWarning->setVisible(false);

    connect(mFileChangedWarning, &FileChangedWarning::reload, this, &DocumentManager::reloadCurrentDocument);
    connect(mFileChangedWarning, &FileChangedWarning::ignore, this, &DocumentManager::hideChangedWarning);

    QVBoxLayout *vertical = new QVBoxLayout(mWidget);
    vertical->addWidget(mTabBar);
    vertical->addWidget(mFileChangedWarning);
    vertical->setMargin(0);
    vertical->setSpacing(0);

    mEditorStack = new QStackedLayout;
    mEditorStack->addWidget(mNoEditorWidget);
    vertical->addLayout(mEditorStack);

    connect(mTabBar, &QTabBar::currentChanged,
            this, &DocumentManager::currentIndexChanged);
    connect(mTabBar, &QTabBar::tabCloseRequested,
            this, &DocumentManager::documentCloseRequested);
    connect(mTabBar, &QTabBar::tabMoved,
            this, &DocumentManager::documentTabMoved);
    connect(mTabBar, SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(tabContextMenuRequested(QPoint)));

    connect(mFileSystemWatcher, &FileSystemWatcher::fileChanged,
            this, &DocumentManager::fileChanged);

    connect(TilesetManager::instance(), &TilesetManager::tilesetImagesChanged,
            this, &DocumentManager::tilesetImagesChanged);
}

DocumentManager::~DocumentManager()
{
    // All documents should be closed gracefully beforehand
    Q_ASSERT(mDocuments.isEmpty());
    Q_ASSERT(mTilesetDocuments.isEmpty());
    Q_ASSERT(mTilesetToDocument.isEmpty());
    delete mWidget;
}

QWidget *DocumentManager::widget() const
{
    return mWidget;
}

void DocumentManager::setEditor(Document::DocumentType documentType, Editor *editor)
{
    Q_ASSERT(!mEditorForType.contains(documentType));
    mEditorForType.insert(documentType, editor);
    mEditorStack->addWidget(editor->editorWidget());

    if (MapEditor *mapEditor = qobject_cast<MapEditor*>(editor))
        mMapEditor = mapEditor;
}

Editor *DocumentManager::editor(Document::DocumentType documentType) const
{
    return mEditorForType.value(documentType);
}

void DocumentManager::deleteEditor(Document::DocumentType documentType)
{
    Q_ASSERT(mEditorForType.contains(documentType));
    Editor *editor = mEditorForType.take(documentType);
    if (editor == mMapEditor)
        mMapEditor = nullptr;
    delete editor;
}

Editor *DocumentManager::currentEditor() const
{
    if (Document *document = currentDocument())
        return editor(document->type());

    return nullptr;
}

void DocumentManager::saveState()
{
    QHashIterator<Document::DocumentType, Editor*> iterator(mEditorForType);
    while (iterator.hasNext())
        iterator.next().value()->saveState();
}

void DocumentManager::restoreState()
{
    QHashIterator<Document::DocumentType, Editor*> iterator(mEditorForType);
    while (iterator.hasNext())
        iterator.next().value()->restoreState();
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

bool DocumentManager::switchToDocument(Document *document)
{
    const int index = mDocuments.indexOf(document);
    if (index != -1) {
        switchToDocument(index);
        return true;
    }

    return false;
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

void DocumentManager::openFile()
{
    emit fileOpenRequested();
}

void DocumentManager::openFile(const QString &path)
{
    emit fileOpenRequested(path);
}

void DocumentManager::saveFile()
{
    emit fileSaveRequested();
}

void DocumentManager::addDocument(Document *document)
{
    Q_ASSERT(document);
    Q_ASSERT(!mDocuments.contains(document));

    mDocuments.append(document);
    mUndoGroup->addStack(document->undoStack());

    if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document)) {
        for (const SharedTileset &tileset : mapDocument->map()->tilesets())
            addToTilesetDocument(tileset, mapDocument);
    } else if (TilesetDocument *tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
        // We may have opened a bare tileset that wasn't seen before
        if (!mTilesetDocuments.contains(tilesetDocument)) {
            mTilesetToDocument.insert(tilesetDocument->tileset(), tilesetDocument);
            mTilesetDocuments.append(tilesetDocument);
            emit tilesetDocumentAdded(tilesetDocument);
        }
    }

    if (!document->fileName().isEmpty())
        mFileSystemWatcher->addPath(document->fileName());

    if (Editor *editor = mEditorForType.value(document->type()))
        editor->addDocument(document);

    const int documentIndex = mDocuments.size() - 1;

    mTabBar->addTab(document->displayName());
    mTabBar->setTabToolTip(documentIndex, document->fileName());

    // todo: updateDocumentTab if an embedded tileset name changes
    connect(document, SIGNAL(fileNameChanged(QString,QString)),
            SLOT(fileNameChanged(QString,QString)));
    connect(document, SIGNAL(modifiedChanged()), SLOT(modifiedChanged()));
    connect(document, SIGNAL(saved()), SLOT(documentSaved()));

    if (auto *mapDocument = qobject_cast<MapDocument*>(document)) {
        connect(mapDocument, &MapDocument::tilesetAdded, this, &DocumentManager::tilesetAdded);
        connect(mapDocument, &MapDocument::tilesetRemoved, this, &DocumentManager::tilesetRemoved);
        connect(mapDocument, &MapDocument::tilesetReplaced, this, &DocumentManager::tilesetReplaced);
    }

    if (auto *tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
        connect(tilesetDocument, &TilesetDocument::tilesetNameChanged, this, &DocumentManager::tilesetNameChanged);
    }

    switchToDocument(documentIndex);

    // todo: fix this (move to MapEditor)
    //    centerViewOn(0, 0);
}

/**
 * Returns whether the given document has unsaved modifications. For map files
 * with embedded tilesets, that includes checking whether any of the embedded
 * tilesets have unsaved modifications.
 */
bool DocumentManager::isDocumentModified(Document *document) const
{
    if (auto mapDocument = qobject_cast<MapDocument*>(document)) {
        for (const SharedTileset &tileset : mapDocument->map()->tilesets()) {
            if (TilesetDocument *tilesetDocument = findTilesetDocument(tileset))
                if (tilesetDocument->isEmbedded() && tilesetDocument->isModified())
                    return true;
        }
    }

    return document->isModified();
}

/**
 * Returns whether the given document was changed on disk. Taking into account
 * the case where the given document is an embedded tileset document.
 */
bool DocumentManager::isDocumentChangedOnDisk(Document *document) const
{
    if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
        if (tilesetDocument->isEmbedded())
            document = tilesetDocument->mapDocuments().first();
    }

    return mDocumentsChangedOnDisk.contains(document);
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

    if (Editor *editor = mEditorForType.value(document->type()))
        editor->removeDocument(document);

    if (!document->fileName().isEmpty()) {
        mFileSystemWatcher->removePath(document->fileName());
        mDocumentsChangedOnDisk.remove(document);
    }

    if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document)) {
        for (const SharedTileset &tileset : mapDocument->map()->tilesets())
            removeFromTilesetDocument(tileset, mapDocument);

        delete document;
    } else if (TilesetDocument *tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
        if (tilesetDocument->mapDocuments().isEmpty()) {
            mTilesetToDocument.remove(tilesetDocument->tileset());
            mTilesetDocuments.removeOne(tilesetDocument);
            emit tilesetDocumentRemoved(tilesetDocument);
            delete document;
        } else {
            tilesetDocument->disconnect(this);
        }
    }
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
    Document *oldDocument = mDocuments.at(index);
    QString error;

    if (auto mapDocument = qobject_cast<MapDocument*>(oldDocument)) {
        // TODO: Consider fixing the reload to avoid recreating the MapDocument
        auto newDocument = MapDocument::load(oldDocument->fileName(),
                                        mapDocument->readerFormat(),
                                        &error);
        if (!newDocument) {
            emit reloadError(tr("%1:\n\n%2").arg(oldDocument->fileName(), error));
            return false;
        }

        // Replace old tab
        addDocument(newDocument);
        closeDocumentAt(index);
        mTabBar->moveTab(mDocuments.size() - 1, index);

        checkTilesetColumns(mapDocument);

    } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(oldDocument)) {
        if (tilesetDocument->isEmbedded()) {
            // For embedded tilesets, we need to reload the map
            index = mDocuments.indexOf(tilesetDocument->mapDocuments().first());
            if (!reloadDocumentAt(index))
                return false;
        } else if (!tilesetDocument->reload(&error)) {
            emit reloadError(tr("%1:\n\n%2").arg(oldDocument->fileName(), error));
            return false;
        }

        mDocumentsChangedOnDisk.remove(tilesetDocument);
    }

    if (!isDocumentChangedOnDisk(currentDocument()))
        mFileChangedWarning->setVisible(false);

    return true;
}

void DocumentManager::closeAllDocuments()
{
    while (!mDocuments.isEmpty())
        closeCurrentDocument();
}

void DocumentManager::currentIndexChanged()
{
    Document *document = currentDocument();
    Editor *editor = nullptr;
    bool changed = false;

    if (document) {
        editor = mEditorForType.value(document->type());
        mUndoGroup->setActiveStack(document->undoStack());

        changed = isDocumentChangedOnDisk(document);
    }

    if (editor) {
        editor->setCurrentDocument(document);
        mEditorStack->setCurrentWidget(editor->editorWidget());
    } else {
        mEditorStack->setCurrentWidget(mNoEditorWidget);
    }

    mFileChangedWarning->setVisible(changed);

    emit currentDocumentChanged(document);
}

void DocumentManager::fileNameChanged(const QString &fileName,
                                      const QString &oldFileName)
{
    if (!fileName.isEmpty())
        mFileSystemWatcher->addPath(fileName);
    if (!oldFileName.isEmpty())
        mFileSystemWatcher->removePath(oldFileName);

    // Update the tabs for all opened embedded tilesets
    Document *document = static_cast<Document*>(sender());
    if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document)) {
        for (const SharedTileset &tileset : mapDocument->map()->tilesets()) {
            if (TilesetDocument *tilesetDocument = findTilesetDocument(tileset))
                updateDocumentTab(tilesetDocument);
        }
    }

    updateDocumentTab(document);
}

void DocumentManager::modifiedChanged()
{
    updateDocumentTab(static_cast<Document*>(sender()));
}

void DocumentManager::updateDocumentTab(Document *document)
{
    const int index = mDocuments.indexOf(document);
    if (index == -1)
        return;

    QString tabText = document->displayName();
    if (document->isModified())
        tabText.prepend(QLatin1Char('*'));

    mTabBar->setTabText(index, tabText);
    mTabBar->setTabToolTip(index, document->fileName());
}

void DocumentManager::documentSaved()
{
    Document *document = static_cast<Document*>(sender());

    if (mDocumentsChangedOnDisk.remove(document)) {
        if (!isDocumentModified(currentDocument()))
            mFileChangedWarning->setVisible(false);
    }
}

void DocumentManager::documentTabMoved(int from, int to)
{
    mDocuments.move(from, to);
}

void DocumentManager::tabContextMenuRequested(const QPoint &pos)
{
    int index = mTabBar->tabAt(pos);
    if (index == -1)
        return;

    QMenu menu(mTabBar->window());

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

    menu.exec(mTabBar->mapToGlobal(pos));
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

void DocumentManager::tilesetNameChanged(Tileset *tileset)
{
    auto *tilesetDocument = findTilesetDocument(tileset->sharedPointer());
    if (tilesetDocument->isEmbedded())
        updateDocumentTab(tilesetDocument);
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
    if (!isDocumentModified(document)) {
        reloadDocumentAt(index);
        return;
    }

    mDocumentsChangedOnDisk.insert(document);

    if (isDocumentChangedOnDisk(currentDocument()))
        mFileChangedWarning->setVisible(true);
}

void DocumentManager::hideChangedWarning()
{
    Document *document = currentDocument();
    if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document)) {
        if (tilesetDocument->isEmbedded())
            document = tilesetDocument->mapDocuments().first();
    }

    mDocumentsChangedOnDisk.remove(document);
    mFileChangedWarning->setVisible(false);
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

TilesetDocument *DocumentManager::findTilesetDocument(const SharedTileset &tileset) const
{
    return mTilesetToDocument.value(tileset);
}

TilesetDocument *DocumentManager::findTilesetDocument(const QString &fileName) const
{
    const QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    if (canonicalFilePath.isEmpty()) // file doesn't exist
        return nullptr;

    for (auto tilesetDocument : mTilesetDocuments) {
        QString name = tilesetDocument->fileName();
        if (!name.isEmpty() && QFileInfo(name).canonicalFilePath() == canonicalFilePath)
            return tilesetDocument;
    }

    return nullptr;
}

/**
 * Searches for a document for the given tileset, creating it if it does not
 * exist already.
 */
TilesetDocument *DocumentManager::findOrCreateTilesetDocument(const SharedTileset &tileset)
{
    auto tilesetDocument = findTilesetDocument(tileset);

    // Create TilesetDocument instance when it doesn't exist yet
    if (!tilesetDocument) {
        tilesetDocument = new TilesetDocument(tileset);
        mTilesetToDocument.insert(tileset, tilesetDocument);
        mTilesetDocuments.append(tilesetDocument);
        emit tilesetDocumentAdded(tilesetDocument);
    }

    return tilesetDocument;
}

void DocumentManager::openTileset(const SharedTileset &tileset)
{
    auto tilesetDocument = findTilesetDocument(tileset);
    Q_ASSERT(tilesetDocument);

    if (!switchToDocument(tilesetDocument))
        addDocument(tilesetDocument);
}

void DocumentManager::addToTilesetDocument(const SharedTileset &tileset, MapDocument *mapDocument)
{
    auto tilesetDocument = findOrCreateTilesetDocument(tileset);
    tilesetDocument->addMapDocument(mapDocument);
}

void DocumentManager::removeFromTilesetDocument(const SharedTileset &tileset, MapDocument *mapDocument)
{
    TilesetDocument *tilesetDocument = findTilesetDocument(tileset);
    Q_ASSERT(tilesetDocument);

    tilesetDocument->removeMapDocument(mapDocument);

    bool unused = tilesetDocument->mapDocuments().isEmpty();
    bool external = tilesetDocument->tileset()->isExternal();
    int index = mDocuments.indexOf(tilesetDocument);

    // Delete the TilesetDocument instance when its tileset is no longer reachable
    if (unused && !(index >= 0 && external)) {
        if (index != -1) {
            closeDocumentAt(index);
        } else {
            mTilesetToDocument.remove(tileset);
            mTilesetDocuments.removeOne(tilesetDocument);
            emit tilesetDocumentRemoved(tilesetDocument);
            delete tilesetDocument;
        }
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
    if (tileset.expectedRowCount() < 2 || tileset.rowCount() < 2)
        return false;

    return true;
}

void DocumentManager::tilesetImagesChanged(Tileset *tileset)
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
        bool tilesetAdjusted = false;

        for (Document *document : mDocuments) {
            if (MapDocument *mapDocument = qobject_cast<MapDocument*>(document)) {
                Map *map = mapDocument->map();

                if (map->tilesets().contains(sharedTileset)) {
                    auto command1 = new AdjustTileIndexes(mapDocument, *tileset);
                    mapDocument->undoStack()->beginMacro(command1->text());
                    mapDocument->undoStack()->push(command1);

                    if (!tilesetAdjusted) {
                        TilesetDocument *tilesetDocument = findTilesetDocument(sharedTileset);
                        Q_ASSERT(tilesetDocument);

                        auto command2 = new AdjustTileMetaData(tilesetDocument);
                        tilesetAdjusted = true;
                        mapDocument->undoStack()->push(command2);
                    }

                    mapDocument->undoStack()->endMacro();
                }
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

            TilesetDocument *tilesetDocument = findTilesetDocument(tileset);
            Q_ASSERT(tilesetDocument);
            auto command2 = new AdjustTileMetaData(tilesetDocument);

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
