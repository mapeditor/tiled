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

#include <QList>
#include <QObject>
#include <QPair>
#include <QPointF>
#include <QSet>

class QTabWidget;
class QUndoGroup;
class QStackedLayout;
class QTabBar;

namespace Tiled {

class FileSystemWatcher;

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

    /**
     * Returns the document manager widget. It contains the different map views
     * and a tab bar to switch between them.
     */
    QWidget *widget() const;

    void setEditor(Document::DocumentType documentType, Editor *editor);
    Editor *editor(Document::DocumentType documentType) const;
    void deleteEditor(Document::DocumentType documentType);
    QList<Editor*> editors() const;

    Editor *currentEditor() const;

    void saveState();
    void restoreState();

    /**
     * Returns the undo group that combines the undo stacks of all opened map
     * documents.
     *
     * @see Document::undoStack()
     */
    QUndoGroup *undoGroup() const { return mUndoGroup; }

    /**
     * Returns the current map document, or 0 when there is none.
     */
    Document *currentDocument() const;

    /**
     * Returns the map view of the current document, or 0 when there is none.
     */
    MapView *currentMapView() const;

    MapView *viewForDocument(MapDocument *mapDocument) const;

    /**
     * Returns the number of map documents.
     */
    int documentCount() const { return mDocuments.size(); }

    /**
     * Searches for a document with the given \a fileName and returns its
     * index. Returns -1 when the document isn't open.
     */
    int findDocument(const QString &fileName) const;

    /**
     * Switches to the map document at the given \a index.
     */
    void switchToDocument(int index);
    bool switchToDocument(Document *document);

    /**
     * Adds the new or opened \a document to the document manager.
     */
    void addDocument(Document *document);

    bool isDocumentModified(Document *document) const;
    bool isDocumentChangedOnDisk(Document *document) const;

    bool saveDocument(Document *document, const QString &fileName);
    bool saveDocumentAs(Document *document);

    /**
     * Closes the current map document. Will not ask the user whether to save
     * any changes!
     */
    void closeCurrentDocument();

    /**
     * Closes all documents except the one pointed to by index.
     */
    void closeOtherDocuments(int index);

    /**
     * Closes all documents whose tabs are to the right of the index.
     */
    void closeDocumentsToRight(int index);

    /**
     * Closes the document at the given \a index. Will not ask the user whether
     * to save any changes!
     */
    void closeDocumentAt(int index);

    /**
     * Reloads the current document. Will not ask the user whether to save any
     * changes!
     *
     * \sa reloadDocumentAt()
     */
    bool reloadCurrentDocument();

    /**
     * Reloads the document at the given \a index. It will lose any undo
     * history and current selections. Will not ask the user whether to save
     * any changes!
     *
     * Returns whether the map loaded successfully.
     */
    bool reloadDocumentAt(int index);

    /**
     * Close all documents. Will not ask the user whether to save any changes!
     */
    void closeAllDocuments();

    void checkTilesetColumns(MapDocument *mapDocument);

    /**
     * Returns all open map documents.
     */
    const QList<Document*> &documents() const { return mDocuments; }

    TilesetDocumentsModel *tilesetDocumentsModel() const;

    TilesetDocument *findTilesetDocument(const SharedTileset &tileset) const;
    TilesetDocument *findTilesetDocument(const QString &fileName) const;

    /**
     * Opens the document for the given \a tileset.
     */
    void openTileset(const SharedTileset &tileset);

    /**
     * Centers the current map on the tile coordinates \a x, \a y.
     */
    void centerMapViewOn(qreal x, qreal y);
    void centerMapViewOn(const QPointF &pos)
    { centerMapViewOn(pos.x(), pos.y()); }

    /**
     * Unsets a flag to stop closeOtherDocuments() and closeDocumentsToRight()
     * when Cancel is pressed
     */
    void abortMultiDocumentClose();

signals:
    void fileOpenRequested();
    void fileOpenRequested(const QString &path);
    void fileSaveRequested();

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
    ~DocumentManager();

    bool askForAdjustment(const Tileset &tileset);

    void addToTilesetDocument(const SharedTileset &tileset, MapDocument *mapDocument);
    void removeFromTilesetDocument(const SharedTileset &tileset, MapDocument *mapDocument);

    bool eventFilter(QObject *object, QEvent *event) override;

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

inline TilesetDocumentsModel *DocumentManager::tilesetDocumentsModel() const
{
    return mTilesetDocumentsModel;
}

} // namespace Tiled::Internal
} // namespace Tiled
