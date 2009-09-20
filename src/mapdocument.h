/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef MAPDOCUMENT_H
#define MAPDOCUMENT_H

#include <QObject>

class QPoint;
class QRect;
class QRegion;
class QSize;
class QUndoStack;

namespace Tiled {

class Map;
class MapObject;
class Tileset;

namespace Internal {

class LayerModel;
class MapRenderer;
class TileSelectionModel;

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
    MapDocument(Map *map);

    /**
     * Destructor.
     */
    ~MapDocument();

    /**
     * Returns the map instance. Be aware that directly modifying the map will
     * not allow the GUI to update itself appropriately.
     */
    Map *map() const { return mMap; }

    /**
     * Sets the current layer to the given index.
     */
    void setCurrentLayer(int index);

    /**
     * Returns the index of the currently selected layer. Returns -1 if no
     * layer is currently selected.
     */
    int currentLayer() const;

    /**
     * Resize this map to the given \a size, while at the same time shifting
     * the contents by \a offset.
     */
    void resizeMap(const QSize &size, const QPoint &offset);

    enum LayerType {
        TileLayerType,
        ObjectLayerType
    };
    void addLayer(LayerType layerType, const QString &name);
    void duplicateLayer();
    void moveLayerUp(int index);
    void moveLayerDown(int index);
    void removeLayer(int index);

    /**
     * Adds a tileset to this map. Emits the appropriate signal.
     */
    void addTileset(Tileset *tileset);

    /**
     * Returns the layer model. Can be used to modify the layer stack of the
     * map, and to display the layer stack in a view.
     */
    LayerModel *layerModel() const { return mLayerModel; }

    /**
     * Returns the selection model.
     */
    TileSelectionModel *selectionModel() const { return mSelectionModel; }

    /**
     * Returns the map renderer.
     */
    MapRenderer *renderer() const { return mRenderer; }

    /**
     * Returns the undo stack of this map document. Should be used to push any
     * commands on that modify the map.
     */
    QUndoStack *undoStack() const { return mUndoStack; }

    void emitMapChanged();

    /**
     * Emits the region changed signal for the specified region. The region
     * should be in tile coordinates. This method is used by the TilePainter.
     */
    void emitRegionChanged(const QRegion &region);

    void emitObjectsAdded(const QList<MapObject*> &objects);
    void emitObjectsRemoved(const QList<MapObject*> &objects);
    void emitObjectsChanged(const QList<MapObject*> &objects);

    inline void emitObjectAdded(MapObject *object)
    { emitObjectsAdded(QList<MapObject*>() << object); }

    inline void emitObjectRemoved(MapObject *object)
    { emitObjectsRemoved(QList<MapObject*>() << object); }

    inline void emitObjectChanged(MapObject *object)
    { emitObjectsChanged(QList<MapObject*>() << object); }

    /**
     * Converts the given rectangle from tile to pixel coordinates.
     * TODO: Move to MapRenderer (and adapt return type)
     */
    QRect toPixelCoordinates(const QRect &r) const;

    /**
     * Snaps the given point to the tile grid of this map document.
     * TODO: Move to MapRenderer
     */
    QPoint snapToTileGrid(const QPoint &p) const;

signals:
    /**
     * Emitted when the map size or its tile size changes.
     */
    void mapChanged();

    void layerAdded(int index);
    void layerRemoved(int index);
    void layerChanged(int index);

    /**
     * Emitted when the current layer changes.
     */
    void currentLayerChanged(int index);

    /**
     * Emitted when a certain region of the map changes. The region is given in
     * tile coordinates.
     */
    void regionChanged(const QRegion &region);

    /**
     * Emitted when a tileset is added to this map.
     */
    void tilesetAdded(Tileset *tileset);

    void objectsAdded(const QList<MapObject*> &objects);
    void objectsRemoved(const QList<MapObject*> &objects);
    void objectsChanged(const QList<MapObject*> &objects);

private slots:
    void onLayerAdded(int index);
    void onLayerRemoved(int index);

private:
    Map *mMap;
    LayerModel *mLayerModel;
    TileSelectionModel *mSelectionModel;
    MapRenderer *mRenderer;
    int mCurrentLayer;
    QUndoStack *mUndoStack;
};

} // namespace Internal
} // namespace Tiled

#endif // MAPDOCUMENT_H
