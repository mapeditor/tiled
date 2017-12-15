/*
 * documentmanager.h
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

#pragma once

#include "document.h"
#include "tileset.h"

#include <QHash>
#include <QList>
#include <QObject>
#include <QPointF>
#include <QSet>

class QTabWidget;
class QUndoGroup;
class QStackedLayout;
class QTabBar;

namespace Tiled {

class FileSystemWatcher;
class ObjectTemplate;

namespace Internal {

class AbstractTool;
class BrokenLinksModel;
class BrokenLinksWidget;
class Document;
class Editor;
class FileChangedWarning;
class MapDocument;
class MapEditor;
class MapScene;
class MapView;
class TilesetDocument;
class TilesetDocumentsModel;

/**
 * This class controls the open documents.
 */
class DocumentManager : public QObject
{
    Q_OBJECT

public:
    static DocumentManager *instance();
    static void deleteInstance();

    QWidget *widget() const;

    void setEditor(Document::DocumentType documentType, Editor *editor);
    Editor *editor(Document::DocumentType documentType) const;
    void deleteEditor(Document::DocumentType documentType);
    QList<Editor*> editors() const;

    Editor *currentEditor() const;

    void saveState();
    void restoreState();

    QUndoGroup *undoGroup() const;

    Document *currentDocument() const;

    MapView *currentMapView() const;
    MapView *viewForDocument(MapDocument *mapDocument) const;

    int documentCount() const;

    int findDocument(const QString &fileName) const;

    void switchToDocument(int index);
    bool switchToDocument(Document *document);

    void addDocument(Document *document);

    bool isDocumentModified(Document *document) const;
    bool isDocumentChangedOnDisk(Document *document) const;

    Document *loadDocument(const QString &fileName,
                           FileFormat *fileFormat = nullptr,
                           QString *error = nullptr);

    bool saveDocument(Document *document, const QString &fileName);
    bool saveDocumentAs(Document *document);

    void closeCurrentDocument();
    void closeAllDocuments();

    void closeOtherDocuments(int index);
    void closeDocumentsToRight(int index);
    void closeDocumentAt(int index);

    bool reloadCurrentDocument();
    bool reloadDocumentAt(int index);

    void checkTilesetColumns(MapDocument *mapDocument);

    const QList<Document*> &documents() const;

    TilesetDocumentsModel *tilesetDocumentsModel() const;

    TilesetDocument *findTilesetDocument(const SharedTileset &tileset) const;
    TilesetDocument *findTilesetDocument(const QString &fileName) const;

    void openTileset(const SharedTileset &tileset);

    void centerMapViewOn(qreal x, qreal y);
    void centerMapViewOn(const QPointF &pos)
    { centerMapViewOn(pos.x(), pos.y()); }

    void abortMultiDocumentClose();

    void addReference(Document *document);
    void removeReference(Document *document);

signals:
    void fileOpenRequested();
    void fileOpenRequested(const QString &path);
    void fileSaveRequested();
    void templateOpenRequested(const QString &path);
    void templateTilesetReplaced();

    /**
     * Emitted when the current displayed map document changed.
     */
    void currentDocumentChanged(Document *document);

    /**
     * Emitted when the user requested the document at \a index to be closed.
     */
    void documentCloseRequested(int index);

    /**
     * Emitted when a document is about to be closed.
     */
    void documentAboutToClose(Document *document);

    /**
     * Emitted when an error occurred while reloading the map.
     */
    void reloadError(const QString &error);

    void tilesetDocumentAdded(TilesetDocument *tilesetDocument);
    void tilesetDocumentRemoved(TilesetDocument *tilesetDocument);

public slots:
    void switchToLeftDocument();
    void switchToRightDocument();

    void openFile();
    void openFile(const QString &path);
    void saveFile();

private slots:
    void currentIndexChanged();
    void fileNameChanged(const QString &fileName,
                         const QString &oldFileName);
    void modifiedChanged();
    void updateDocumentTab(Document *document);
    void documentSaved();
    void documentTabMoved(int from, int to);
    void tabContextMenuRequested(const QPoint &pos);

    void tilesetAdded(int index, Tileset *tileset);
    void tilesetRemoved(Tileset *tileset);
    void tilesetReplaced(int index, Tileset *tileset, Tileset *oldTileset);

    void tilesetNameChanged(Tileset *tileset);

    void fileChanged(const QString &fileName);
    void hideChangedWarning();

    void tilesetImagesChanged(Tileset *tileset);

private:
    DocumentManager(QObject *parent = nullptr);
    ~DocumentManager() override;

    bool askForAdjustment(const Tileset &tileset);

    void addToTilesetDocument(const SharedTileset &tileset, MapDocument *mapDocument);
    void removeFromTilesetDocument(const SharedTileset &tileset, MapDocument *mapDocument);

    bool eventFilter(QObject *object, QEvent *event) override;

    QHash<Document*, int> mReferencedDocuments;

    QList<Document*> mDocuments;
    TilesetDocumentsModel *mTilesetDocumentsModel;

    QWidget *mWidget;
    QWidget *mNoEditorWidget;
    QTabBar *mTabBar;
    FileChangedWarning *mFileChangedWarning;
    BrokenLinksModel *mBrokenLinksModel;
    BrokenLinksWidget *mBrokenLinksWidget;
    QStackedLayout *mEditorStack;
    MapEditor *mMapEditor;

    QHash<Document::DocumentType, Editor*> mEditorForType;

    QUndoGroup *mUndoGroup;
    FileSystemWatcher *mFileSystemWatcher;
    QSet<Document*> mDocumentsChangedOnDisk;

    QMap<SharedTileset, TilesetDocument*> mTilesetToDocument;

    static DocumentManager *mInstance;

    bool mMultiDocumentClose;
};

/**
 * Returns the undo group that combines the undo stacks of all opened
 * documents.
 *
 * @see Document::undoStack()
 */
inline QUndoGroup *DocumentManager::undoGroup() const
{
    return mUndoGroup;
}

/**
 * Returns the number of open documents.
 */
inline int DocumentManager::documentCount() const
{
    return mDocuments.size();
}

/**
 * Returns all open documents.
 */
inline const QList<Document *> &DocumentManager::documents() const
{
    return mDocuments;
}

inline TilesetDocumentsModel *DocumentManager::tilesetDocumentsModel() const
{
    return mTilesetDocumentsModel;
}


template <typename DocumentType>
class DocumentRef
{
public:
    DocumentRef(DocumentType *document)
        : mDocument(document)
    {
        Q_ASSERT(document);
        DocumentManager::instance()->addReference(mDocument);
    }

    DocumentRef(const DocumentRef &ref)
        : mDocument(ref.mDocument)
    {
        DocumentManager::instance()->addReference(mDocument);
    }

    ~DocumentRef()
    {
        DocumentManager::instance()->removeReference(mDocument);
    }

    DocumentRef &operator=(const DocumentRef &ref)
    {
        DocumentManager::instance()->addReference(ref.mDocument);
        DocumentManager::instance()->removeReference(mDocument);
        mDocument = ref.mDocument;
        return *this;
    }

    DocumentType *document() const { return mDocument; }

private:
    DocumentType *mDocument;
};

typedef DocumentRef<MapDocument> MapDocumentRef;
typedef DocumentRef<TilesetDocument> TilesetDocumentRef;

} // namespace Tiled::Internal
} // namespace Tiled
