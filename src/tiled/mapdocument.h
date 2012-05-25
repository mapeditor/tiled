/*
 * mapdocument.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Jeff Bland <jeff@teamphobic.com>
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

#ifndef MAPDOCUMENT_H
#define MAPDOCUMENT_H

#include <QList>
#include <QObject>
#include <QRegion>
#include <QString>

#include "layer.h"

class QPoint;
class QRect;
class QSize;
class QUndoStack;

namespace Tiled {

class Map;
class MapObject;
class MapRenderer;
class Tileset;

namespace Internal {

class LayerModel;
class TileSelectionModel;
class MapObjectModel;

/**
 * Represents an editable map. The purpose of this class is to make sure that
 * any editing operations will cause the appropriate signals to be emitted, in
 * order to allow the GUI to update accordingly.
 *
 * At the moment the map document provides the layer model, keeps track of the
 * the currently selected layer and provides an API for adding and removing
 * map objects. It also owns the QUndoStack.
 */
class MapDocument : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructs a map document around the given map. The map document takes
     * ownership of the map.
     */
    MapDocument(Map *map, const QString &fileName = QString());

    /**
     * Destructor.
     */
    ~MapDocument();

    /**
     * Saves the map to its current file name. Returns whether or not the file
     * was saved successfully. If not, <i>error</i> will be set to the error
     * message if it is not 0.
     */
    bool save(QString *error = 0);

    /**
     * Saves the map to the file at \a fileName. Returns whether or not the
     * file was saved successfully. If not, <i>error</i> will be set to the
     * error message if it is not 0.
     *
     * If the save was succesful, the file name of this document will be set
     * to \a fileName.
     */
    bool save(const QString &fileName, QString *error = 0);

    QString fileName() const { return mFileName; }
    void setFileName(const QString &fileName);

    QString displayName() const;

    bool isModified() const;

    /**
     * Returns the map instance. Be aware that directly modifying the map will
     * not allow the GUI to update itself appropriately.
     */
    Map *map() const { return mMap; }

    /**
     * Sets the current layer to the given index.
     */
    void setCurrentLayerIndex(int index);

    /**
     * Returns the index of the currently selected layer. Returns -1 if no
     * layer is currently selected.
     */
    int currentLayerIndex() const { return mCurrentLayerIndex; }

    /**
     * Returns the currently selected layer, or 0 if no layer is currently
     * selected.
     */
    Layer *currentLayer() const;

    /**
     * Resize this map to the given \a size, while at the same time shifting
     * the contents by \a offset.
     */
    void resizeMap(const QSize &size, const QPoint &offset);

    /**
     * Offsets the layers at \a layerIndexes by \a offset, within \a bounds,
     * and optionally wraps on the X or Y axis.
     */
    void offsetMap(const QList<int> &layerIndexes,
                   const QPoint &offset,
                   const QRect &bounds,
                   bool wrapX, bool wrapY);


    void addLayer(Layer::Type layerType);
    void duplicateLayer();
    void mergeLayerDown();
    void moveLayerUp(int index);
    void moveLayerDown(int index);
    void removeLayer(int index);
    void toggleOtherLayers(int index);

    void insertTileset(int index, Tileset *tileset);
    void removeTilesetAt(int index);
    void moveTileset(int from, int to);
    void setTilesetFileName(Tileset *tileset, const QString &fileName);
    void setTilesetName(Tileset *tileset, const QString &name);

    /**
     * Returns the layer model. Can be used to modify the layer stack of the
     * map, and to display the layer stack in a view.
     */
    LayerModel *layerModel() const { return mLayerModel; }

    MapObjectModel *mapObjectModel() const { return mMapObjectModel; }

    /**
     * Returns the map renderer.
     */
    MapRenderer *renderer() const { return mRenderer; }

    /**
     * Returns the undo stack of this map document. Should be used to push any
     * commands on that modify the map.
     */
    QUndoStack *undoStack() const { return mUndoStack; }

    /**
     * Returns the selected area of tiles.
     */
    const QRegion &tileSelection() const { return mTileSelection; }

    /**
     * Sets the selected area of tiles.
     */
    void setTileSelection(const QRegion &selection);

    /**
     * Returns the list of selected objects.
     */
    const QList<MapObject*> &selectedObjects() const
    { return mSelectedObjects; }

    /**
     * Sets the list of selected objects, emitting the selectedObjectsChanged
     * signal.
     */
    void setSelectedObjects(const QList<MapObject*> &selectedObjects);

    /**
     * Makes sure the all tilesets which are used at the given \a map will be
     * present in the map document.
     *
     * To reach the aim, all similar tilesets will be replaced by the version
     * in the current map document and all missing tilesets will be added to
     * the current map document.
     *
     * \warning This method assumes that the tilesets in \a map are managed by
     *          the TilesetManager!
     */
    void unifyTilesets(Map *map);

    void emitMapChanged();

    /**
     * Emits the region changed signal for the specified region. The region
     * should be in tile coordinates. This method is used by the TilePainter.
     */
    void emitRegionChanged(const QRegion &region);

    /**
     * Emits the region edited signal for the specified region and tile layer.
     * The region should be in tile coordinates. This should be called from
     * all map document changing classes which are triggered by user input.
     */
    void emitRegionEdited(const QRegion &region, Layer *layer);

    /**
     * Emits the editLayerNameRequested signal, to get renamed.
     */
    inline void emitEditLayerNameRequested()
    { emit editLayerNameRequested(); }

signals:
    void fileNameChanged();
    void modifiedChanged();

    /**
     * Emitted when the selected tile region changes. Sends the currently
     * selected region and the previously selected region.
     */
    void tileSelectionChanged(const QRegion &newSelection,
                              const QRegion &oldSelection);

    /**
     * Emitted when the list of selected objects changes.
     */
    void selectedObjectsChanged();

    /**
     * Emitted when the map size or its tile size changes.
     */
    void mapChanged();

    void layerAdded(int index);
    void layerAboutToBeRemoved(int index);
    void layerRenamed(int index);
    void layerRemoved(int index);
    void layerChanged(int index);

    /**
     * Emitted after a new layer was added and the name should be edited.
     * Applies to the current layer.
     */
    void editLayerNameRequested();

    /**
     * Emitted when the current layer index changes.
     */
    void currentLayerIndexChanged(int index);

    /**
     * Emitted when a certain region of the map changes. The region is given in
     * tile coordinates.
     */
    void regionChanged(const QRegion &region);

    /**
     * Emitted when a certain region of the map was edited by user input.
     * The region is given in tile coordinates.
     * If multiple layers have been edited, multiple signals will be emitted.
     */
    void regionEdited(const QRegion &region, Layer *layer);

    void tilesetAdded(int index, Tileset *tileset);
    void tilesetRemoved(Tileset *tileset);
    void tilesetMoved(int from, int to);
    void tilesetFileNameChanged(Tileset *tileset);
    void tilesetNameChanged(Tileset *tileset);

    void objectsAdded(const QList<MapObject*> &objects);
    void objectsAboutToBeRemoved(const QList<MapObject*> &objects);
    void objectsRemoved(const QList<MapObject*> &objects);
    void objectsChanged(const QList<MapObject*> &objects);

private slots:
    void onObjectsRemoved(const QList<MapObject*> &objects);

    void onLayerAdded(int index);
    void onLayerAboutToBeRemoved(int index);
    void onLayerRemoved(int index);

private:
    void deselectObjects(const QList<MapObject*> &objects);

    QString mFileName;
    Map *mMap;
    LayerModel *mLayerModel;
    QRegion mTileSelection;
    QList<MapObject*> mSelectedObjects;
    MapRenderer *mRenderer;
    int mCurrentLayerIndex;
    MapObjectModel *mMapObjectModel;
    QUndoStack *mUndoStack;
};

} // namespace Internal
} // namespace Tiled

#endif // MAPDOCUMENT_H
