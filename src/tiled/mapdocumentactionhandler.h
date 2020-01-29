/*
 * mapdocumentactionhandler.h
 * Copyright 2010-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#pragma once

#include <QObject>
#include "mapobject.h"

class QAction;
class QMenu;

namespace Tiled {

class ObjectGroup;
class MapObject;

class MapDocument;

/**
 * The map document action handler deals with most basic actions that can be
 * performed on a MapDocument.
 */
class MapDocumentActionHandler : public QObject
{
    Q_OBJECT

    enum LayerViaVariant {
        ViaCopy,
        ViaCut,
    };

public:
    explicit MapDocumentActionHandler(QObject *parent = nullptr);
    ~MapDocumentActionHandler() override;

    static MapDocumentActionHandler *instance() { return mInstance; }

    void retranslateUi();

    void setMapDocument(MapDocument *mapDocument);
    MapDocument *mapDocument() const { return mMapDocument; }

    QAction *actionSelectAll() const { return mActionSelectAll; }
    QAction *actionSelectInverse() const { return mActionSelectInverse; }
    QAction *actionSelectNone() const { return mActionSelectNone; }
    QAction *actionCropToSelection() const { return mActionCropToSelection; }
    QAction *actionAutocrop() const { return mActionAutocrop; }

    QAction *actionAddTileLayer() const { return mActionAddTileLayer; }
    QAction *actionAddObjectGroup() const { return mActionAddObjectGroup; }
    QAction *actionAddImageLayer() const { return mActionAddImageLayer; }
    QAction *actionAddGroupLayer() const { return mActionAddGroupLayer; }
    QAction *actionLayerViaCopy() const { return mActionLayerViaCopy; }
    QAction *actionLayerViaCut() const { return mActionLayerViaCut; }
    QAction *actionGroupLayers() const { return mActionGroupLayers; }
    QAction *actionUngroupLayers() const { return mActionUngroupLayers; }

    QAction *actionDuplicateLayers() const { return mActionDuplicateLayers; }
    QAction *actionMergeLayersDown() const { return mActionMergeLayersDown; }
    QAction *actionRemoveLayers() const { return mActionRemoveLayers; }
    QAction *actionSelectPreviousLayer() const { return mActionSelectPreviousLayer; }
    QAction *actionSelectNextLayer() const { return mActionSelectNextLayer; }
    QAction *actionMoveLayersUp() const { return mActionMoveLayersUp; }
    QAction *actionMoveLayersDown() const { return mActionMoveLayersDown; }
    QAction *actionToggleSelectedLayers() const { return mActionToggleSelectedLayers; }
    QAction *actionToggleLockSelectedLayers() const { return mActionToggleLockSelectedLayers; }
    QAction *actionToggleOtherLayers() const { return mActionToggleOtherLayers; }
    QAction *actionToggleLockOtherLayers() const { return mActionToggleLockOtherLayers; }
    QAction *actionLayerProperties() const { return mActionLayerProperties; }

    QAction *actionDuplicateObjects() const { return mActionDuplicateObjects; }
    QAction *actionRemoveObjects() const { return mActionRemoveObjects; }

    QMenu *createNewLayerMenu(QWidget *parent) const;
    QMenu *createGroupLayerMenu(QWidget *parent) const;

public slots:
    void cut();
    bool copy();
    void delete_(); // 'delete' is a reserved word

    void selectAll();
    void selectInverse();
    void selectNone();

    void copyPosition();

    void cropToSelection();
    void autocrop();

    void addTileLayer();
    void addObjectGroup();
    void addImageLayer();
    void addGroupLayer();
    void layerViaCopy() { layerVia(ViaCopy); }
    void layerViaCut() { layerVia(ViaCut); }
    void layerVia(LayerViaVariant variant);
    void groupLayers();
    void ungroupLayers();

    void duplicateLayers();
    void mergeLayersDown();
    void selectPreviousLayer();
    void selectNextLayer();
    void moveLayersUp();
    void moveLayersDown();
    void removeLayers();
    void toggleSelectedLayers();
    void toggleLockSelectedLayers();
    void toggleOtherLayers();
    void toggleLockOtherLayers();
    void layerProperties();

    void duplicateObjects();
    void removeObjects();
    void moveObjectsToGroup(ObjectGroup *);

    void selectAllInstances(const ObjectTemplate *objectTemplate);

private:
    void updateActions();

    MapDocument *mMapDocument;

    QAction *mActionSelectAll;
    QAction *mActionSelectInverse;
    QAction *mActionSelectNone;
    QAction *mActionCropToSelection;
    QAction *mActionAutocrop;

    QAction *mActionAddTileLayer;
    QAction *mActionAddObjectGroup;
    QAction *mActionAddImageLayer;
    QAction *mActionAddGroupLayer;
    QAction *mActionLayerViaCopy;
    QAction *mActionLayerViaCut;
    QAction *mActionGroupLayers;
    QAction *mActionUngroupLayers;

    QAction *mActionDuplicateLayers;
    QAction *mActionMergeLayersDown;
    QAction *mActionRemoveLayers;
    QAction *mActionSelectPreviousLayer;
    QAction *mActionSelectNextLayer;
    QAction *mActionMoveLayersUp;
    QAction *mActionMoveLayersDown;
    QAction *mActionToggleSelectedLayers;
    QAction *mActionToggleLockSelectedLayers;
    QAction *mActionToggleOtherLayers;
    QAction *mActionToggleLockOtherLayers;
    QAction *mActionLayerProperties;

    QAction *mActionDuplicateObjects;
    QAction *mActionRemoveObjects;

    static MapDocumentActionHandler *mInstance;
};

} // namespace Tiled
