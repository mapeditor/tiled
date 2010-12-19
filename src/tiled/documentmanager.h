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
    DocumentManager(QObject *parent = 0);
    ~DocumentManager();

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
     * Returns the number of map documents.
     */
    int documentCount() const { return mDocuments.size(); }

    /**
     * Switches to the map document at the given \a index.
     */
    void switchToDocument(int index);

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
     * Close all documents. Will not ask the user whether to save any changes!
     */
    void closeAllDocuments();

    /**
     * Returns all open map documents.
     */
    QList<MapDocument*> documents() const { return mDocuments; }

signals:
    /**
     * Emitted when the current displayed map document changed.
     */
    void currentDocumentChanged(MapDocument *mapDocument);

    /**
     * Emitted when the user requested the document at \a index to be closed.
     */
    void documentCloseRequested(int index);

private slots:
    void currentIndexChanged();
    void setSelectedTool(AbstractTool *tool);
    void documentFileNameChanged();

private:
    QList<MapDocument*> mDocuments;

    QTabWidget *mTabWidget;
    QUndoGroup *mUndoGroup;
    AbstractTool *mSelectedTool;
    MapScene *mSceneWithTool;
    QString mUntitledFileName;
};

} // namespace Tiled::Internal
} // namespace Tiled

#endif // DOCUMENT_MANAGER_H
