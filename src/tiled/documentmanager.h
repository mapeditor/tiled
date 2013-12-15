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

#ifndef DOCUMENT_MANAGER_H
#define DOCUMENT_MANAGER_H

#include "mapview.h"
#include "mapscene.h"
#include "mapdocument.h"

#include <QList>
#include <QPair>

class QTabWidget;
class QUndoGroup;

namespace Tiled {

class Tileset;

namespace Internal {

class MapDocument;
class MapScene;
class MapView;

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

    /**
     * Returns the undo group that combines the undo stacks of all opened map
     * documents.
     *
     * @see MapDocument::undoStack()
     */
    QUndoGroup *undoGroup() const { return mUndoGroup; }

    /**
     * Returns the current map document, or 0 when there is none.
     */
    MapDocument *currentDocument() const;

    /**
     * Returns the map view of the current document, or 0 when there is none.
     */
    MapView *currentMapView() const;

    /**
     * Returns the map scene of the current document, or 0 when there is none.
     */
    MapScene *currentMapScene() const;

    /**
     * Returns the map view that displays the given document, or 0 when there
     * is none.
     */
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
    void switchToDocument(MapDocument *mapDocument);

    /**
     * Adds the new or opened \a mapDocument to the document manager.
     */
    void addDocument(MapDocument *mapDocument);

    /**
     * Closes the current map document. Will not ask the user whether to save
     * any changes!
     */
    void closeCurrentDocument();

    /**
     * Closes the document at the given \a index. Will not ask the user whether
     * to save any changes!
     */
    void closeDocumentAt(int index);

    /**
     * Close all documents. Will not ask the user whether to save any changes!
     */
    void closeAllDocuments();

    /**
     * Returns all open map documents.
     */
    QList<MapDocument*> documents() const { return mDocuments; }

    /**
     * Centers the current map on the tile coordinates \a x, \a y.
     */
    void centerViewOn(qreal x, qreal y);
    void centerViewOn(const QPointF &pos)
    { centerViewOn(pos.x(), pos.y()); }

signals:
    /**
     * Emitted when the current displayed map document changed.
     */
    void currentDocumentChanged(MapDocument *mapDocument);

    /**
     * Emitted when the user requested the document at \a index to be closed.
     */
    void documentCloseRequested(int index);

public slots:
    void switchToLeftDocument();
    void switchToRightDocument();

    void setSelectedTool(AbstractTool *tool);

private slots:
    void currentIndexChanged();
    void updateDocumentTab();

private:
    DocumentManager(QObject *parent = 0);
    ~DocumentManager();

    QList<MapDocument*> mDocuments;

    QTabWidget *mTabWidget;
    QUndoGroup *mUndoGroup;
    AbstractTool *mSelectedTool;
    MapScene *mSceneWithTool;

    static DocumentManager *mInstance;
};

} // namespace Tiled::Internal
} // namespace Tiled

#endif // DOCUMENT_MANAGER_H
