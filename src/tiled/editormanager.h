/*
 * editormanager.h
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

#pragma once

#include "tilededitor_global.h"
#include "mapdocument.h"
#include "tilesetdocument.h"

#include <QHash>
#include <QObject>
#include <QVector>

class QUndoGroup;

namespace Tiled {

class FileSystemWatcher;
class Document;
class WorldDocument;
class Editor;
class MapDocument;
class TilesetDocument;

/**
 * Manages the open documents (asset session) independently of the GUI.
 * Exists in both GUI and CLI mode, enabling asset manipulation (open, close,
 * save, reload) from scripts regardless of how Tiled is launched.
 *
 * DocumentManager (GUI-only) delegates to EditorManager and adds tabs, editors,
 * and dialogs on top.
 */
class TILED_EDITOR_EXPORT EditorManager : public QObject
{
    Q_OBJECT

    EditorManager(QObject *parent = nullptr);
    ~EditorManager() override;

    friend int main(int argc, char *argv[]);
    friend class Document;

public:
    static EditorManager *instance();
    static void deleteInstance();

    QUndoGroup *undoGroup() const;
    FileSystemWatcher *fileSystemWatcher() const { return mFileSystemWatcher; }

    Document *currentDocument() const;
    void setCurrentDocument(Document *document);
    void setCurrentDocument(int index);

    const QVector<DocumentPtr> &documents() const;

    int findDocument(const QString &fileName) const;
    int findDocument(Document *document) const;
    Document *documentForFileName(const QString &fileName) const;

    void addDocument(const DocumentPtr &document);
    int insertDocument(int index, const DocumentPtr &document);

    DocumentPtr loadDocument(const QString &fileName,
                             FileFormat *fileFormat = nullptr,
                             QString *error = nullptr);

    bool saveDocument(Document *document);
    bool saveDocument(Document *document, const QString &fileName, QString *error = nullptr);

    void closeDocumentAt(int index);
    void closeDocument(Document *document);
    void closeAllDocuments();
    void moveDocument(int from, int to);

    bool reloadDocument(Document *document);

    bool isDocumentModified(Document *document) const;

    TilesetDocument *findTilesetDocument(const SharedTileset &tileset) const;
    TilesetDocument *findTilesetDocument(const QString &fileName) const;

    TilesetDocument *openTileset(const SharedTileset &tileset);

    void addToTilesetDocument(const SharedTileset &tileset, MapDocument *mapDocument);
    void removeFromTilesetDocument(const SharedTileset &tileset, MapDocument *mapDocument);

signals:
    void documentCreated(Document *document);
    void documentOpened(Document *document);
    void documentReloaded(Document *document);
    void documentAboutToBeSaved(Document *document);
    void documentSaved(Document *document);
    void documentAboutToClose(Document *document);
    void documentClosed(Document *document);

    void currentDocumentChanged(Document *document);
    void documentsChanged();

    void tilesetDocumentAdded(TilesetDocument *tilesetDocument);
    void tilesetDocumentRemoved(TilesetDocument *tilesetDocument);

private:
    void registerDocument(Document *document);
    void unregisterDocument(Document *document);

    void setCurrentIndex(int index);

    QVector<DocumentPtr> mDocuments;
    QVector<TilesetDocumentPtr> mTilesetDocuments;
    int mCurrentIndex = -1;

    QUndoGroup *mUndoGroup;
    FileSystemWatcher *mFileSystemWatcher;
    QHash<QString, Document*> mDocumentByFileName;

    static EditorManager *mInstance;
};

inline QUndoGroup *EditorManager::undoGroup() const
{
    return mUndoGroup;
}

inline const QVector<DocumentPtr> &EditorManager::documents() const
{
    return mDocuments;
}

inline bool EditorManager::saveDocument(Document *document)
{
    return saveDocument(document, document->fileName());
}

} // namespace Tiled
