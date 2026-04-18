/*
 * editormanager.cpp
 * Copyright 2025
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

#include "editormanager.h"

#include "containerhelpers.h"
#include "document.h"
#include "filesystemwatcher.h"
#include "map.h"
#include "mapdocument.h"
#include "mapformat.h"
#include "pluginmanager.h"
#include "tilesetdocument.h"
#include "tilesetformat.h"
#include "worlddocument.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QUndoGroup>
#include <QUndoStack>

using namespace Tiled;

EditorManager *EditorManager::mInstance = nullptr;

EditorManager *EditorManager::instance()
{
    if (!mInstance)
        mInstance = new EditorManager();
    return mInstance;
}

EditorManager::EditorManager(QObject *parent)
    : QObject(parent)
    , mUndoGroup(new QUndoGroup(this))
    , mFileSystemWatcher(new FileSystemWatcher(this))
{
}

EditorManager::~EditorManager()
{
    mInstance = nullptr;
}

void EditorManager::deleteInstance()
{
    delete mInstance;
    mInstance = nullptr;
}

Document *EditorManager::currentDocument() const
{
    if (mCurrentIndex < 0 || mCurrentIndex >= mDocuments.size())
        return nullptr;
    return mDocuments.at(mCurrentIndex).data();
}

void EditorManager::setCurrentDocument(Document *document)
{
    const int index = findDocument(document);
    if (index != -1)
        setCurrentDocument(index);
}

void EditorManager::setCurrentDocument(int index)
{
    if (index < 0 || index >= mDocuments.size())
        index = -1;
    if (mCurrentIndex != index) {
        mCurrentIndex = index;
        emit currentDocumentChanged(currentDocument());
    }
}

void EditorManager::setCurrentIndex(int index)
{
    setCurrentDocument(index);
}

int EditorManager::findDocument(const QString &fileName) const
{
    const QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    if (canonicalFilePath.isEmpty())
        return -1;

    for (int i = 0; i < mDocuments.size(); ++i) {
        if (mDocuments.at(i)->canonicalFilePath() == canonicalFilePath)
            return i;
    }
    return -1;
}

int EditorManager::findDocument(Document *document) const
{
    return indexOf(mDocuments, document);
}

Document *EditorManager::documentForFileName(const QString &fileName) const
{
    const QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    if (canonicalFilePath.isEmpty())
        return nullptr;
    return mDocumentByFileName.value(canonicalFilePath);
}

void EditorManager::addDocument(const DocumentPtr &document)
{
    insertDocument(mDocuments.size(), document);
    setCurrentDocument(mDocuments.size() - 1);
}

int EditorManager::insertDocument(int index, const DocumentPtr &document)
{
    Q_ASSERT(document);
    Q_ASSERT(!mDocuments.contains(document));

    mDocuments.insert(index, document);
    mUndoGroup->addStack(document->undoStack());

    Document *documentPtr = document.data();

    if (auto mapDocument = qobject_cast<MapDocument*>(documentPtr)) {
        for (const SharedTileset &tileset : mapDocument->map()->tilesets())
            addToTilesetDocument(tileset, mapDocument);
    } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(documentPtr)) {
        if (!mTilesetDocuments.contains(tilesetDocument->sharedFromThis())) {
            mTilesetDocuments.append(tilesetDocument->sharedFromThis());
            emit tilesetDocumentAdded(tilesetDocument);
        }
    }

    emit documentOpened(documentPtr);
    emit documentsChanged();

    return index;
}

DocumentPtr EditorManager::loadDocument(const QString &fileName,
                                        FileFormat *fileFormat,
                                        QString *error)
{
    const QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    if (Document *doc = mDocumentByFileName.value(canonicalFilePath))
        return doc->sharedFromThis();

    if (!fileFormat) {
        fileFormat = PluginManager::find<FileFormat>([&](FileFormat *format) {
            return format->hasCapabilities(FileFormat::Read) && format->supportsFile(fileName);
        });
    }

    if (!fileFormat) {
        if (error)
            *error = QCoreApplication::translate("Tiled::EditorManager", "Unrecognized file format.");
        return DocumentPtr();
    }

    DocumentPtr document;

    if (MapFormat *mapFormat = qobject_cast<MapFormat*>(fileFormat)) {
        document = MapDocument::load(fileName, mapFormat, error);
    } else if (TilesetFormat *tilesetFormat = qobject_cast<TilesetFormat*>(fileFormat)) {
        if (auto tilesetDocument = findTilesetDocument(fileName)) {
            document = tilesetDocument->sharedFromThis();
        } else {
            document = TilesetDocument::load(fileName, tilesetFormat, error);
        }
    }

    return document;
}

bool EditorManager::saveDocument(Document *document, const QString &fileName, QString *error)
{
    if (fileName.isEmpty())
        return false;

    emit documentAboutToBeSaved(document);

    QString err;
    if (!document->save(fileName, &err)) {
        setCurrentDocument(document);
        if (error)
            *error = err;
        return false;
    }

    emit documentSaved(document);
    return true;
}

void EditorManager::closeDocumentAt(int index)
{
    if (index < 0 || index >= mDocuments.size())
        return;

    auto document = mDocuments.at(index);
    emit documentAboutToClose(document.data());

    mDocuments.removeAt(index);
    document->disconnect(this);

    if (auto mapDocument = qobject_cast<MapDocument*>(document.data())) {
        for (const SharedTileset &tileset : mapDocument->map()->tilesets())
            removeFromTilesetDocument(tileset, mapDocument);
    } else if (auto tilesetDocument = qobject_cast<TilesetDocument*>(document.data())) {
        if (tilesetDocument->mapDocuments().isEmpty()) {
            mTilesetDocuments.removeAll(tilesetDocument->sharedFromThis());
            emit tilesetDocumentRemoved(tilesetDocument);
        }
    }

    emit documentClosed(document.data());
    emit documentsChanged();

    if (mCurrentIndex >= mDocuments.size())
        mCurrentIndex = mDocuments.size() - 1;
    if (mCurrentIndex >= 0)
        emit currentDocumentChanged(currentDocument());
    else
        emit currentDocumentChanged(nullptr);
}

void EditorManager::closeDocument(Document *document)
{
    const int index = findDocument(document);
    if (index != -1)
        closeDocumentAt(index);
}

void EditorManager::closeAllDocuments()
{
    while (!mDocuments.isEmpty())
        closeDocumentAt(mDocuments.size() - 1);
}

void EditorManager::moveDocument(int from, int to)
{
    if (from < 0 || from >= mDocuments.size() || to < 0 || to >= mDocuments.size() || from == to)
        return;
    mDocuments.move(from, to);
    if (mCurrentIndex == from)
        mCurrentIndex = to;
    else if (mCurrentIndex > from && mCurrentIndex <= to)
        --mCurrentIndex;
    else if (mCurrentIndex >= to && mCurrentIndex < from)
        ++mCurrentIndex;
    emit documentsChanged();
}

bool EditorManager::reloadDocument(Document *document)
{
    QString error;

    switch (document->type()) {
    case Document::MapDocumentType: {
        auto mapDocument = static_cast<MapDocument*>(document);
        if (!mapDocument->reload(&error))
            return false;
        break;
    }
    case Document::TilesetDocumentType: {
        auto tilesetDocument = static_cast<TilesetDocument*>(document);
        if (tilesetDocument->isEmbedded()) {
            if (!reloadDocument(tilesetDocument->mapDocuments().first()))
                return false;
        } else if (!tilesetDocument->reload(&error)) {
            return false;
        }
        tilesetDocument->setChangedOnDisk(false);
        break;
    }
    case Document::WorldDocumentType: {
        auto worldDocument = static_cast<WorldDocument*>(document);
        if (!worldDocument->reload(&error))
            return false;
        break;
    }
    case Document::ProjectDocumentType:
        break;
    }

    emit documentReloaded(document);
    return true;
}

bool EditorManager::isDocumentModified(Document *document) const
{
    if (auto mapDocument = qobject_cast<MapDocument*>(document)) {
        for (const SharedTileset &tileset : mapDocument->map()->tilesets()) {
            if (const auto tilesetDocument = findTilesetDocument(tileset))
                if (tilesetDocument->isEmbedded() && tilesetDocument->isModified())
                    return true;
        }
    }
    return document->isModified();
}

TilesetDocument *EditorManager::findTilesetDocument(const SharedTileset &tileset) const
{
    return TilesetDocument::findDocumentForTileset(tileset);
}

TilesetDocument *EditorManager::findTilesetDocument(const QString &fileName) const
{
    const QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();
    if (canonicalFilePath.isEmpty())
        return nullptr;

    for (const auto &tilesetDocument : mTilesetDocuments) {
        if (!tilesetDocument->fileName().isEmpty() &&
            QFileInfo(tilesetDocument->fileName()).canonicalFilePath() == canonicalFilePath)
            return tilesetDocument.data();
    }
    return nullptr;
}

TilesetDocument *EditorManager::openTileset(const SharedTileset &tileset)
{
    TilesetDocumentPtr tilesetDocument;
    if (auto existingTilesetDocument = findTilesetDocument(tileset))
        tilesetDocument = existingTilesetDocument->sharedFromThis();
    else
        tilesetDocument = TilesetDocumentPtr::create(tileset);

    if (!mDocuments.contains(tilesetDocument))
        addDocument(tilesetDocument);
    else
        setCurrentDocument(tilesetDocument.data());

    return tilesetDocument.data();
}

void EditorManager::addToTilesetDocument(const SharedTileset &tileset, MapDocument *mapDocument)
{
    if (auto existingTilesetDocument = findTilesetDocument(tileset)) {
        existingTilesetDocument->addMapDocument(mapDocument);
    } else {
        auto tilesetDocument = TilesetDocumentPtr::create(tileset);
        tilesetDocument->addMapDocument(mapDocument);
        mTilesetDocuments.append(tilesetDocument);
        emit tilesetDocumentAdded(tilesetDocument.data());
    }
}

void EditorManager::removeFromTilesetDocument(const SharedTileset &tileset, MapDocument *mapDocument)
{
    auto tilesetDocument = findTilesetDocument(tileset);
    if (!tilesetDocument)
        return;

    auto tilesetDocumentPtr = tilesetDocument->sharedFromThis();
    tilesetDocument->removeMapDocument(mapDocument);

    const bool unused = tilesetDocument->mapDocuments().isEmpty();
    const bool external = tilesetDocument->tileset()->isExternal();
    const int index = findDocument(tilesetDocument);

    if (unused && !(index >= 0 && external)) {
        if (index != -1) {
            closeDocumentAt(index);
        } else {
            mTilesetDocuments.removeAll(tilesetDocumentPtr);
            emit tilesetDocumentRemoved(tilesetDocument);
        }
    }
}

void EditorManager::registerDocument(Document *document)
{
    const QString &canonicalPath = document->canonicalFilePath();
    if (canonicalPath.isEmpty())
        return;

    mFileSystemWatcher->addPath(canonicalPath);

    const auto i = mDocumentByFileName.constFind(canonicalPath);
    if (i != mDocumentByFileName.constEnd()) {
        qWarning() << "Document already registered:" << canonicalPath;
        return;
    }
    mDocumentByFileName.insert(canonicalPath, document);
}

void EditorManager::unregisterDocument(Document *document)
{
    const QString &canonicalPath = document->canonicalFilePath();
    if (canonicalPath.isEmpty())
        return;

    mFileSystemWatcher->removePath(canonicalPath);

    const auto i = mDocumentByFileName.constFind(canonicalPath);
    if (i != mDocumentByFileName.constEnd() && *i == document)
        mDocumentByFileName.erase(i);
}

#include "moc_editormanager.cpp"
