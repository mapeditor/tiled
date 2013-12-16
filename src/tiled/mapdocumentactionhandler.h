/*
 * mapdocumentactionhandler.h
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2011, Stefan Beller <stefanbeller@googlemail.com
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

#ifndef MAPDOCUMENTACTIONHANDLER_H
#define MAPDOCUMENTACTIONHANDLER_H

#include <QObject>

class QAction;

namespace Tiled {

class ObjectGroup;

namespace Internal {

class MapDocument;

/**
 * The map document action handler deals with most basic actions that can be
 * performed on a MapDocument.
 */
class MapDocumentActionHandler : public QObject
{
    Q_OBJECT

public:
    explicit MapDocumentActionHandler(QObject *parent = 0);
    ~MapDocumentActionHandler();

    static MapDocumentActionHandler *instance() { return mInstance; }

    void retranslateUi();

    void setMapDocument(MapDocument *mapDocument);
    MapDocument *mapDocument() const { return mMapDocument; }

    QAction *actionSelectAll() const { return mActionSelectAll; }
    QAction *actionSelectNone() const { return mActionSelectNone; }
    QAction *actionCropToSelection() const { return mActionCropToSelection; }

    QAction *actionAddTileLayer() const { return mActionAddTileLayer; }
    QAction *actionAddObjectGroup() const { return mActionAddObjectGroup; }
    QAction *actionAddImageLayer() const { return mActionAddImageLayer; }
    QAction *actionDuplicateLayer() const { return mActionDuplicateLayer; }
    QAction *actionMergeLayerDown() const { return mActionMergeLayerDown; }
    QAction *actionRemoveLayer() const { return mActionRemoveLayer; }
    QAction *actionSelectPreviousLayer() const
    { return mActionSelectPreviousLayer; }
    QAction *actionSelectNextLayer() const { return mActionSelectNextLayer; }
    QAction *actionMoveLayerUp() const { return mActionMoveLayerUp; }
    QAction *actionMoveLayerDown() const { return mActionMoveLayerDown; }
    QAction *actionToggleOtherLayers() const
    { return mActionToggleOtherLayers; }
    QAction *actionLayerProperties() const { return mActionLayerProperties; }

    QAction *actionDuplicateObjects() const { return mActionDuplicateObjects; }
    QAction *actionRemoveObjects() const { return mActionRemoveObjects; }

signals:
    void mapDocumentChanged(MapDocument *mapDocument);

public slots:
    void selectAll();
    void selectNone();

    void copyPosition();

    void cropToSelection();

    void addTileLayer();
    void addObjectGroup();
    void addImageLayer();
    void duplicateLayer();
    void mergeLayerDown();
    void selectPreviousLayer();
    void selectNextLayer();
    void moveLayerUp();
    void moveLayerDown();
    void removeLayer();
    void toggleOtherLayers();
    void layerProperties();

    void duplicateObjects();
    void removeObjects();
    void moveObjectsToGroup(ObjectGroup *);

private slots:
    void updateActions();

private:
    MapDocument *mMapDocument;

    QAction *mActionSelectAll;
    QAction *mActionSelectNone;
    QAction *mActionCropToSelection;

    QAction *mActionAddTileLayer;
    QAction *mActionAddObjectGroup;
    QAction *mActionAddImageLayer;
    QAction *mActionDuplicateLayer;
    QAction *mActionMergeLayerDown;
    QAction *mActionRemoveLayer;
    QAction *mActionSelectPreviousLayer;
    QAction *mActionSelectNextLayer;
    QAction *mActionMoveLayerUp;
    QAction *mActionMoveLayerDown;
    QAction *mActionToggleOtherLayers;
    QAction *mActionLayerProperties;

    QAction *mActionDuplicateObjects;
    QAction *mActionRemoveObjects;

    static MapDocumentActionHandler *mInstance;
};

} // namespace Internal
} // namespace Tiled

#endif // MAPDOCUMENTACTIONHANDLER_H
