/*
 * mapdocument.h
 * Copyright 2008-2017, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#pragma once

#include "document.h"
#include "layer.h"
#include "mapformat.h"
#include "tiled.h"
#include "tileset.h"

#include <QList>
#include <QPointer>
#include <QRegion>

class QModelIndex;
class QPoint;
class QRect;
class QSize;
class QUndoStack;

namespace Tiled {

class Map;
class MapObject;
class MapRenderer;
class ObjectTemplate;
class Terrain;
class Tile;
class WangSet;

namespace Internal {

class LayerModel;
class MapObjectModel;
class TileSelectionModel;

/**
 * Represents an editable map. The purpose of this class is to make sure that
 * any editing operations will cause the appropriate signals to be emitted, in
 * order to allow the GUI to update accordingly.
 *
 * The map document provides the layer model, keeps track of the currently
 * selected layer and provides an API for adding and removing map objects. It
 * also owns the QUndoStack.
 */
class MapDocument : public Document
{
    Q_OBJECT

public:
    enum TileLayerChangeFlag {
        LayerDrawMarginsChanged,
        LayerBoundsChanged
    };
    Q_DECLARE_FLAGS(TileLayerChangeFlags, TileLayerChangeFlag)
    Q_FLAG(TileLayerChangeFlags)

    /**
     * Constructs a map document around the given map. The map document takes
     * ownership of the map.
     */
    MapDocument(Map *map, const QString &fileName = QString());

    ~MapDocument() override;

    bool save(const QString &fileName, QString *error = nullptr) override;

    /**
     * Loads a map and returns a MapDocument instance on success. Returns null
     * on error and sets the \a error message.
     */
    static MapDocument *load(const QString &fileName,
                             MapFormat *format,
                             QString *error = nullptr);

    MapFormat *readerFormat() const;
    void setReaderFormat(MapFormat *format);

    FileFormat *writerFormat() const override;
    void setWriterFormat(MapFormat *format);

    MapFormat *exportFormat() const override;
    void setExportFormat(FileFormat *format) override;

    QString displayName() const override;

    /**
     * Returns the map instance. Be aware that directly modifying the map will
     * not allow the GUI to update itself appropriately.
     */
    Map *map() const { return mMap; }

    int layerIndex(const Layer *layer) const;

    /**
     * Returns the currently selected layer, or 0 if no layer is currently
     * selected.
     */
    Layer *currentLayer() const { return mCurrentLayer; }
    void setCurrentLayer(Layer *layer);

    /**
     * Resize this map to the given \a size, while at the same time shifting
     * the contents by \a offset. If \a removeObjects is true then all objects
     * which are outside the map will be removed.
     */
    void resizeMap(const QSize &size, const QPoint &offset, bool removeObjects);

    void autocropMap();

    /**
     * Offsets the \a layers by \a offset, within \a bounds, and optionally
     * wraps on the X or Y axis.
     */
    void offsetMap(const QList<Layer *> &layers,
                   const QPoint &offset,
                   const QRect &bounds,
                   bool wrapX, bool wrapY);

    void flipSelectedObjects(FlipDirection direction);
    void rotateSelectedObjects(RotateDirection direction);

    Layer *addLayer(Layer::TypeFlag layerType);
    void groupLayer(Layer *layer);
    void ungroupLayer(Layer *layer);
    void duplicateLayer();
    void mergeLayerDown();
    void moveLayerUp(Layer *layer);
    void moveLayerDown(Layer *layer);
    void removeLayer(Layer *layer);
    void toggleOtherLayers(Layer *layer);

    void insertTileset(int index, const SharedTileset &tileset);
    void removeTilesetAt(int index);
    SharedTileset replaceTileset(int index, const SharedTileset &tileset);

    void replaceObjectTemplate(const ObjectTemplate *oldObjectTemplate,
                               const ObjectTemplate *newObjectTemplate);

    void duplicateObjects(const QList<MapObject*> &objects);
    void removeObjects(const QList<MapObject*> &objects);
    void moveObjectsToGroup(const QList<MapObject*> &objects,
                            ObjectGroup *objectGroup);
    void moveObjectsUp(const QList<MapObject*> &objects);
    void moveObjectsDown(const QList<MapObject*> &objects);
    void detachObjects(const QList<MapObject*> &objects);

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
     * Creates the map renderer. Should be called after changing the map
     * orientation.
     */
    void createRenderer();

    /**
     * Returns the selected area of tiles.
     */
    const QRegion &selectedArea() const { return mSelectedArea; }

    /**
     * Sets the selected area of tiles.
     */
    void setSelectedArea(const QRegion &selection);

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

    QList<Object*> currentObjects() const override;

    MapObject *hoveredMapObject() const { return mHoveredMapObject; }
    void setHoveredMapObject(MapObject *object);

    void unifyTilesets(Map *map);
    void unifyTilesets(Map *map, QVector<SharedTileset> &missingTilesets);

    void emitEditLayerNameRequested();

signals:
    /**
     * Emitted when the selected tile region changes. Sends the currently
     * selected region and the previously selected region.
     */
    void selectedAreaChanged(const QRegion &newSelection,
                             const QRegion &oldSelection);

    /**
     * Emitted when the list of selected objects changes.
     */
    void selectedObjectsChanged();

    void hoveredMapObjectChanged(MapObject *object, MapObject *previous);

    /**
     * Emitted when the map size or its tile size changes.
     */
    void mapChanged();

    void layerAdded(Layer *layer);
    void layerAboutToBeRemoved(GroupLayer *parentLayer, int index);
    void layerRemoved(Layer *layer);
    void layerChanged(Layer *layer);

    /**
     * Emitted after a new layer was added and the name should be edited.
     * Applies to the current layer.
     */
    void editLayerNameRequested();

    /**
     * Emitted when the current layer changes.
     */
    void currentLayerChanged(Layer *layer);

    /**
     * Emitted when a certain \a region of a \a tileLayer changes. The region
     * is given in tile coordinates.
     */
    void regionChanged(const QRegion &region, TileLayer *tileLayer);

    /**
     * Emitted when a certain region of the map was edited by user input.
     * The region is given in tile coordinates.
     * If multiple layers have been edited, multiple signals will be emitted.
     */
    void regionEdited(const QRegion &region, Layer *layer);

    void tileLayerChanged(TileLayer *layer, TileLayerChangeFlags flags);

    /**
     * Should be emitted when changing the color or drawing order of an object
     * group.
     */
    void objectGroupChanged(ObjectGroup *objectGroup);

    /**
     * Should be emitted when changing the image or the transparent color of
     * an image layer.
     */
    void imageLayerChanged(ImageLayer *imageLayer);

    void tilesetAboutToBeAdded(int index);
    void tilesetAdded(int index, Tileset *tileset);
    void tilesetAboutToBeRemoved(int index);
    void tilesetRemoved(Tileset *tileset);
    void tilesetReplaced(int index, Tileset *tileset, Tileset *oldTileset);

    void objectTemplateReplaced(const ObjectTemplate *newObjectTemplate,
                                const ObjectTemplate *oldObjectTemplate);

    void objectsAdded(const QList<MapObject*> &objects);
    void objectsInserted(ObjectGroup *objectGroup, int first, int last);
    void objectsRemoved(const QList<MapObject*> &objects);
    void objectsChanged(const QList<MapObject*> &objects);
    void objectsTypeChanged(const QList<MapObject*> &objects);
    void objectsIndexChanged(ObjectGroup *objectGroup, int first, int last);

    // emitted from the TilesetDocument
    void tilesetNameChanged(Tileset *tileset);
    void tilesetTileOffsetChanged(Tileset *tileset);
    void tileTypeChanged(Tile *tile);
    void tileImageSourceChanged(Tile *tile);

private slots:
    void onObjectsRemoved(const QList<MapObject*> &objects);

    void onMapObjectModelRowsInserted(const QModelIndex &parent, int first, int last);
    void onMapObjectModelRowsInsertedOrRemoved(const QModelIndex &parent, int first, int last);
    void onObjectsMoved(const QModelIndex &parent, int start, int end,
                        const QModelIndex &destination, int row);

    void onLayerAdded(Layer *layer);
    void onLayerAboutToBeRemoved(GroupLayer *groupLayer, int index);
    void onLayerRemoved(Layer *layer);

public slots:
    void updateTemplateInstances(const ObjectTemplate *objectTemplate);
    void selectAllInstances(const ObjectTemplate *objectTemplate);

private:
    void deselectObjects(const QList<MapObject*> &objects);
    void moveObjectIndex(const MapObject *object, int count);

    /*
     * QPointer is used since the formats referenced here may be dynamically
     * added by a plugin, and can also be removed again.
     */
    QPointer<MapFormat> mReaderFormat;
    QPointer<MapFormat> mWriterFormat;
    QPointer<MapFormat> mExportFormat;
    Map *mMap;
    LayerModel *mLayerModel;
    QRegion mSelectedArea;
    QList<MapObject*> mSelectedObjects;
    MapObject *mHoveredMapObject;       /**< Map object with mouse on top. */
    MapRenderer *mRenderer;
    Layer* mCurrentLayer;
    MapObjectModel *mMapObjectModel;
};

} // namespace Internal
} // namespace Tiled

Q_DECLARE_OPERATORS_FOR_FLAGS(Tiled::Internal::MapDocument::TileLayerChangeFlags)
