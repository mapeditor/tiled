/*
 * mapdocument.h
 * Copyright 2008-2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "layer.h"
#include "tiled.h"

#include <QList>
#include <QObject>
#include <QRegion>
#include <QString>

class QModelIndex;
class QPoint;
class QRect;
class QSize;
class QUndoStack;

namespace Tiled {

class Map;
class MapObject;
class MapRenderer;
class Terrain;
class Tile;
class Tileset;

namespace Internal {

class LayerModel;
class MapObjectModel;
class TerrainModel;
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
     * If the save was successful, the file name of this document will be set
     * to \a fileName.
     *
     * The map format will be the same as this map was opened with.
     */
    bool save(const QString &fileName, QString *error = 0);

    QString fileName() const { return mFileName; }

    QString writerPluginFileName() const { return mWriterPluginFileName; }
    void setWriterPluginFileName(const QString &writerPluginFileName)
    { mWriterPluginFileName = writerPluginFileName; }

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

    void flipSelectedObjects(FlipDirection direction);
    void rotateSelectedObjects(RotateDirection direction);

    void addLayer(Layer::TypeFlag layerType);
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
    void setTilesetTileOffset(Tileset *tileset, const QPoint &tileOffset);

    void duplicateObjects(const QList<MapObject*> &objects);
    void removeObjects(const QList<MapObject*> &objects);
    void moveObjectsToGroup(const QList<MapObject*> &objects,
                            ObjectGroup *objectGroup);

    void setProperty(Object *object, const QString &name, const QString &value);
    void setProperties(Object *object, const Properties &properties);
    void removeProperty(Object *object, const QString &name);

    /**
     * Returns the layer model. Can be used to modify the layer stack of the
     * map, and to display the layer stack in a view.
     */
    LayerModel *layerModel() const { return mLayerModel; }

    MapObjectModel *mapObjectModel() const { return mMapObjectModel; }

    TerrainModel *terrainModel() const { return mTerrainModel; }

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

    Object *currentObject() const { return mCurrentObject; }
    void setCurrentObject(Object *object);

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
    void emitRegionChanged(const QRegion &region);
    void emitRegionEdited(const QRegion &region, Layer *layer);
    void emitTilesetChanged(Tileset *tileset);
    void emitTileTerrainChanged(const QList<Tile*> &tiles);
    void emitTileObjectGroupChanged(Tile *tile);
    void emitTileAnimationChanged(Tile *tile);
    void emitObjectGroupChanged(ObjectGroup *objectGroup);
    void emitImageLayerChanged(ImageLayer *imageLayer);
    void emitEditLayerNameRequested();
    void emitEditCurrentObject();

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

    void currentObjectChanged(Object *object);

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

    void editCurrentObject();

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

    /**
     * Emitted when the terrain information for the given list of tiles was
     * changed. All the tiles are guaranteed to be from the same tileset.
     */
    void tileTerrainChanged(const QList<Tile*> &tiles);

    /**
     * Emitted when the object group associated with a tile changes.
     */
    void tileObjectGroupChanged(Tile *tile);

    /**
     * Emitted when the animation of a tile changes.
     */
    void tileAnimationChanged(Tile *tile);

    /**
     * Emitted after the color of an object group has changed.
     */
    void objectGroupChanged(ObjectGroup *objectGroup);

    /**
     * Emitted after the image and/or the transparent color of an image layer
     * have changed.
     */
    void imageLayerChanged(ImageLayer *imageLayer);

    void tilesetAboutToBeAdded(int index);
    void tilesetAdded(int index, Tileset *tileset);
    void tilesetAboutToBeRemoved(int index);
    void tilesetRemoved(Tileset *tileset);
    void tilesetMoved(int from, int to);
    void tilesetFileNameChanged(Tileset *tileset);
    void tilesetNameChanged(Tileset *tileset);
    void tilesetTileOffsetChanged(Tileset *tileset);
    void tilesetChanged(Tileset *tileset);

    void objectsAdded(const QList<MapObject*> &objects);
    void objectsInserted(ObjectGroup *objectGroup, int first, int last);
    void objectsRemoved(const QList<MapObject*> &objects);
    void objectsChanged(const QList<MapObject*> &objects);
    void objectsIndexChanged(ObjectGroup *objectGroup, int first, int last);

    void propertyAdded(Object *object, const QString &name);
    void propertyRemoved(Object *object, const QString &name);
    void propertyChanged(Object *object, const QString &name);
    void propertiesChanged(Object *object);

private slots:
    void onObjectsRemoved(const QList<MapObject*> &objects);

    void onMapObjectModelRowsInserted(const QModelIndex &parent, int first, int last);
    void onMapObjectModelRowsInsertedOrRemoved(const QModelIndex &parent, int first, int last);
    void onObjectsMoved(const QModelIndex &parent, int start, int end,
                        const QModelIndex &destination, int row);

    void onLayerAdded(int index);
    void onLayerAboutToBeRemoved(int index);
    void onLayerRemoved(int index);

    void onTerrainRemoved(Terrain *terrain);

private:
    void setFileName(const QString &fileName);
    void deselectObjects(const QList<MapObject*> &objects);

    QString mFileName;

    /*
     * The filename of a plugin is unique. So it can be used to determine
     * the right plugin to be used for saving the map again.
     * The nameFilter of a plugin can not be used, since it's translatable.
     * The filename of a plugin must not change while maps are open using this
     * plugin.
     */
    QString mWriterPluginFileName;
    Map *mMap;
    LayerModel *mLayerModel;
    QRegion mTileSelection;
    QList<MapObject*> mSelectedObjects;
    Object *mCurrentObject;             /**< Current properties object. */
    MapRenderer *mRenderer;
    int mCurrentLayerIndex;
    MapObjectModel *mMapObjectModel;
    TerrainModel *mTerrainModel;
    QUndoStack *mUndoStack;
};

/**
 * Emits the map changed signal. This signal should be emitted after changing
 * the map size or its tile size.
 */
inline void MapDocument::emitMapChanged()
{
    emit mapChanged();
}

/**
 * Emits the region changed signal for the specified region. The region
 * should be in tile coordinates. This method is used by the TilePainter.
 */
inline void MapDocument::emitRegionChanged(const QRegion &region)
{
    emit regionChanged(region);
}

/**
 * Emits the region edited signal for the specified region and tile layer.
 * The region should be in tile coordinates. This should be called from
 * all map document changing classes which are triggered by user input.
 */
inline void MapDocument::emitRegionEdited(const QRegion &region, Layer *layer)
{
    emit regionEdited(region, layer);
}

/**
 * Emits the signal notifying tileset models about changes to tile terrain
 * information. All the \a tiles need to be from the same tileset.
 */
inline void MapDocument::emitTileTerrainChanged(const QList<Tile *> &tiles)
{
    if (!tiles.isEmpty())
        emit tileTerrainChanged(tiles);
}

/**
 * Emits the signal notifying the TileCollisionEditor about the object group
 * of a tile changing.
 */
inline void MapDocument::emitTileObjectGroupChanged(Tile *tile)
{
    emit tileObjectGroupChanged(tile);
}

/**
 * Emits the signal notifying about the animation of a tile changing.
 */
inline void MapDocument::emitTileAnimationChanged(Tile *tile)
{
    emit tileAnimationChanged(tile);
}

/**
 * Emits the objectGroupChanged signal, should be called when changing the
 * color or drawing order of an object group.
 */
inline void MapDocument::emitObjectGroupChanged(ObjectGroup *objectGroup)
{
    emit objectGroupChanged(objectGroup);
}

/**
 * Emits the imageLayerChanged signal, should be called when changing the
 * image or the transparent color of an image layer.
 */
inline void MapDocument::emitImageLayerChanged(ImageLayer *imageLayer)
{
    emit imageLayerChanged(imageLayer);
}

/**
 * Emits the editLayerNameRequested signal, to get renamed.
 */
inline void MapDocument::emitEditLayerNameRequested()
{
    emit editLayerNameRequested();
}

/**
 * Emits the editCurrentObject signal, which makes the Properties window become
 * visible and take focus.
 */
inline void MapDocument::emitEditCurrentObject()
{
    emit editCurrentObject();
}

} // namespace Internal
} // namespace Tiled

#endif // MAPDOCUMENT_H
