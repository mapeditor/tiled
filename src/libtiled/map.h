/*
 * map.h
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2010, Andrew G. Crowell <overkill9999@gmail.com>
 *
 * This file is part of libtiled.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MAP_H
#define MAP_H

#include "layer.h"
#include "object.h"

#include <QColor>
#include <QList>
#include <QMargins>
#include <QSize>

namespace Tiled {

class Tile;
class Tileset;
class ObjectGroup;

/**
 * A tile map. Consists of a stack of layers, each can be either a TileLayer
 * or an ObjectGroup.
 *
 * It also keeps track of the list of referenced tilesets.
 */
class TILEDSHARED_EXPORT Map : public Object
{
public:
    /**
     * The orientation of the map determines how it should be rendered. An
     * Orthogonal map is using rectangular tiles that are aligned on a
     * straight grid. An Isometric map uses diamond shaped tiles that are
     * aligned on an isometric projected grid. A Hexagonal map uses hexagon
     * shaped tiles that fit into each other by shifting every other row.
     *
     * Only Orthogonal, Isometric and Staggered maps are supported by this
     * version of Tiled.
     */
    enum Orientation {
        Unknown,
        Orthogonal,
        Isometric,
        Staggered
    };

    /**
     * The different formats in which the tile layer data can be stored.
     */
    enum LayerDataFormat {
        XML        = 0,
        Base64     = 1,
        Base64Gzip = 2,
        Base64Zlib = 3,
        CSV        = 4
    };

    /**
     * Constructor, taking map orientation, size and tile size as parameters.
     */
    Map(Orientation orientation,
        int width, int height,
        int tileWidth, int tileHeight);

    /**
     * Copy constructor. Makes sure that a deep-copy of the layers is created.
     */
    Map(const Map &map);

    /**
     * Destructor.
     */
    ~Map();

    /**
     * Returns the orientation of the map.
     */
    Orientation orientation() const { return mOrientation; }

    /**
     * Sets the orientation of the map.
     */
    void setOrientation(Orientation orientation)
    { mOrientation = orientation; }

    /**
     * Returns the width of this map.
     */
    int width() const { return mWidth; }

    /**
     * Sets the width of this map.
     */
    void setWidth(int width) { mWidth = width; }

    /**
     * Returns the height of this map.
     */
    int height() const { return mHeight; }

    /**
     * Sets the height of this map.
     */
    void setHeight(int height) { mHeight = height; }

    /**
     * Returns the size of this map. Provided for convenience.
     */
    QSize size() const { return QSize(mWidth, mHeight); }

    /**
     * Returns the tile width of this map.
     */
    int tileWidth() const { return mTileWidth; }

    /**
     * Sets the width of one tile.
     */
    void setTileWidth(int width) { mTileWidth = width; }

    /**
     * Returns the tile height used by this map.
     */
    int tileHeight() const { return mTileHeight; }

    /**
     * Sets the height of one tile.
     */
    void setTileHeight(int height) { mTileHeight = height; }

    /**
     * Adjusts the draw margins to be at least as big as the given margins.
     * Called from tile layers when their tiles change.
     */
    void adjustDrawMargins(const QMargins &margins);

    /**
     * Returns the margins that have to be taken into account when figuring
     * out which part of the map to repaint after changing some tiles.
     *
     * @see TileLayer::drawMargins
     */
    QMargins drawMargins() const { return mDrawMargins; }

    void recomputeDrawMargins();

    /**
     * Returns the number of layers of this map.
     */
    int layerCount() const
    { return mLayers.size(); }

    /**
     * Convenience function that returns the number of layers of this map that
     * match the given \a type.
     */
    int layerCount(Layer::TypeFlag type) const;

    int tileLayerCount() const
    { return layerCount(Layer::TileLayerType); }

    int objectGroupCount() const
    { return layerCount(Layer::ObjectGroupType); }

    int imageLayerCount() const
    { return layerCount(Layer::ImageLayerType); }

    /**
     * Returns the layer at the specified index.
     */
    Layer *layerAt(int index) const
    { return mLayers.at(index); }

    /**
     * Returns the list of layers of this map. This is useful when you want to
     * use foreach.
     */
    const QList<Layer*> &layers() const { return mLayers; }

    QList<Layer*> layers(Layer::TypeFlag type) const;
    QList<ObjectGroup*> objectGroups() const;
    QList<TileLayer*> tileLayers() const;

    /**
     * Adds a layer to this map.
     */
    void addLayer(Layer *layer);

    /**
     * Returns the index of the layer given by \a layerName, or -1 if no
     * layer with that name is found.
     *
     * The second optional parameter specifies the layer types which are
     * searched.
     */
    int indexOfLayer(const QString &layerName,
                     unsigned layerTypes = Layer::AnyLayerType) const;

    /**
     * Adds a layer to this map, inserting it at the given index.
     */
    void insertLayer(int index, Layer *layer);

    /**
     * Removes the layer at the given index from this map and returns it.
     * The caller becomes responsible for the lifetime of this layer.
     */
    Layer *takeLayerAt(int index);

    /**
     * Adds a tileset to this map. The map does not take ownership over its
     * tilesets, this is merely for keeping track of which tilesets are used by
     * the map, and their saving order.
     *
     * @param tileset the tileset to add
     */
    void addTileset(Tileset *tileset);

    /**
     * Inserts \a tileset at \a index in the list of tilesets used by this map.
     */
    void insertTileset(int index, Tileset *tileset);

    /**
     * Returns the index of the given \a tileset, or -1 if it is not used in
     * this map.
     */
    int indexOfTileset(Tileset *tileset) const;

    /**
     * Removes the tileset at \a index from this map.
     *
     * \warning Does not make sure that this map no longer refers to tiles from
     *          the removed tileset!
     *
     * \sa addTileset
     */
    void removeTilesetAt(int index);

    /**
     * Replaces all tiles from \a oldTileset with tiles from \a newTileset.
     * Also replaces the old tileset with the new tileset in the list of
     * tilesets.
     */
    void replaceTileset(Tileset *oldTileset, Tileset *newTileset);

    /**
     * Returns the number of tilesets of this map.
     */
    int tilesetCount() const { return mTilesets.size(); }

    /**
     * Returns the tileset at the given index.
     */
    Tileset *tilesetAt(int index) const { return mTilesets.at(index); }

    /**
     * Returns the tilesets that the tiles on this map are using.
     */
    const QList<Tileset*> &tilesets() const { return mTilesets; }

    /**
     * Returns the background color of this map.
     */
    const QColor &backgroundColor() const { return mBackgroundColor; }

    /**
     * Sets the background color of this map.
     */
    void setBackgroundColor(QColor color) { mBackgroundColor = color; }

    /**
     * Returns whether the given \a tileset is used by any tile layer of this
     * map.
     */
    bool isTilesetUsed(Tileset *tileset) const;

    /**
     * Creates a new map that contains the given \a layer. The map size will be
     * determined by the size of the layer.
     *
     * The orientation defaults to Unknown and the tile width and height will
     * default to 0. In case this map needs to be rendered, these properties
     * will need to be properly set.
     */
    static Map *fromLayer(Layer *layer);

    LayerDataFormat layerDataFormat() const
    { return mLayerDataFormat; }
    void setLayerDataFormat(LayerDataFormat format)
    { mLayerDataFormat = format; }

private:
    void adoptLayer(Layer *layer);

    Orientation mOrientation;
    int mWidth;
    int mHeight;
    int mTileWidth;
    int mTileHeight;
    QColor mBackgroundColor;
    QMargins mDrawMargins;
    QList<Layer*> mLayers;
    QList<Tileset*> mTilesets;
    LayerDataFormat mLayerDataFormat;
};

/**
 * Helper function that converts the map orientation to a string value. Useful
 * for map writers.
 *
 * @return The map orientation as a lowercase string.
 */
TILEDSHARED_EXPORT QString orientationToString(Map::Orientation);

/**
 * Helper function that converts a string to a map orientation enumerator.
 * Useful for map readers.
 *
 * @return The map orientation matching the given string, or Map::Unknown if
 *         the string is unrecognized.
 */
TILEDSHARED_EXPORT Map::Orientation orientationFromString(const QString &);

} // namespace Tiled

#endif // MAP_H
