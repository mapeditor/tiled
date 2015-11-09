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
#include "tileset.h"

#include <QColor>
#include <QList>
#include <QMargins>
#include <QSize>

namespace Tiled {

class Tile;
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
     */
    enum Orientation {
        Unknown,
        Orthogonal,
        Isometric,
        Staggered,
        Hexagonal
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
     * The order in which tiles are rendered on screen.
     */
    enum RenderOrder {
        RightDown  = 0,
        RightUp    = 1,
        LeftDown   = 2,
        LeftUp     = 3
    };

    /**
     * Which axis is staggered. Only used by the isometric staggered and
     * hexagonal map renderers.
     */
    enum StaggerAxis {
        StaggerX,
        StaggerY
    };

    /**
     * When staggering, specifies whether the odd or the even rows/columns are
     * shifted half a tile right/down. Only used by the isometric staggered and
     * hexagonal map renderers.
     */
    enum StaggerIndex {
        StaggerOdd  = 0,
        StaggerEven = 1
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
     * Returns the render order of the map.
     */
    RenderOrder renderOrder() const { return mRenderOrder; }

    /**
     * Sets the render order of the map.
     */
    void setRenderOrder(RenderOrder renderOrder)
    { mRenderOrder = renderOrder; }

    /**
     * Returns the width of this map in tiles.
     */
    int width() const { return mWidth; }

    /**
     * Sets the width of this map in tiles.
     */
    void setWidth(int width) { mWidth = width; }

    /**
     * Returns the height of this map in tiles.
     */
    int height() const { return mHeight; }

    /**
     * Sets the height of this map in tiles.
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
     * Returns the size of one tile. Provided for convenience.
     */
    QSize tileSize() const { return QSize(mTileWidth, mTileHeight); }

    int hexSideLength() const;
    void setHexSideLength(int hexSideLength);

    StaggerAxis staggerAxis() const;
    void setStaggerAxis(StaggerAxis staggerAxis);

    StaggerIndex staggerIndex() const;
    void setStaggerIndex(StaggerIndex staggerIndex);

    /**
     * Returns the margins that have to be taken into account when figuring
     * out which part of the map to repaint after changing some tiles.
     */
    QMargins drawMargins() const { return mDrawMargins; }

    QMargins computeLayerOffsetMargins() const;

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
    void addTileset(const SharedTileset &tileset);

    /**
     * Convenience function to be used together with Layer::usedTilesets()
     */
    void addTilesets(const QSet<SharedTileset> &tilesets);

    /**
     * Inserts \a tileset at \a index in the list of tilesets used by this map.
     */
    void insertTileset(int index, const SharedTileset &tileset);

    /**
     * Returns the index of the given \a tileset, or -1 if it is not used in
     * this map.
     */
    int indexOfTileset(const SharedTileset &tileset) const;

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
    void replaceTileset(const SharedTileset &oldTileset,
                        const SharedTileset &newTileset);

    /**
     * Returns the number of tilesets of this map.
     */
    int tilesetCount() const { return mTilesets.size(); }

    /**
     * Returns the tileset at the given index.
     */
    SharedTileset tilesetAt(int index) const { return mTilesets.at(index); }

    /**
     * Returns the tilesets that the tiles on this map are using.
     */
    const QVector<SharedTileset> &tilesets() const { return mTilesets; }

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
    bool isTilesetUsed(const Tileset *tileset) const;

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

    void setNextObjectId(int nextId);
    int nextObjectId() const;
    int takeNextObjectId();

private:
    void adoptLayer(Layer *layer);

    Orientation mOrientation;
    RenderOrder mRenderOrder;
    int mWidth;
    int mHeight;
    int mTileWidth;
    int mTileHeight;
    int mHexSideLength;
    StaggerAxis mStaggerAxis;
    StaggerIndex mStaggerIndex;
    QColor mBackgroundColor;
    QMargins mDrawMargins;
    QList<Layer*> mLayers;
    QVector<SharedTileset> mTilesets;
    LayerDataFormat mLayerDataFormat;
    int mNextObjectId;
};


inline int Map::hexSideLength() const
{
    return mHexSideLength;
}

inline void Map::setHexSideLength(int hexSideLength)
{
    mHexSideLength = hexSideLength;
}

inline Map::StaggerAxis Map::staggerAxis() const
{
    return mStaggerAxis;
}

inline void Map::setStaggerAxis(StaggerAxis staggerAxis)
{
    mStaggerAxis = staggerAxis;
}

inline Map::StaggerIndex Map::staggerIndex() const
{
    return mStaggerIndex;
}

inline void Map::setStaggerIndex(StaggerIndex staggerIndex)
{
    mStaggerIndex = staggerIndex;
}

/**
 * Sets the next id to be used for objects on this map.
 */
inline void Map::setNextObjectId(int nextId)
{
    Q_ASSERT(nextId > 0);
    mNextObjectId = nextId;
}

/**
 * Returns the next object id for this map.
 */
inline int Map::nextObjectId() const
{
    return mNextObjectId;
}

/**
 * Returns the next object id for this map and allocates a new one.
 */
inline int Map::takeNextObjectId()
{
    return mNextObjectId++;
}


TILEDSHARED_EXPORT QString staggerAxisToString(Map::StaggerAxis);
TILEDSHARED_EXPORT Map::StaggerAxis staggerAxisFromString(const QString &);

TILEDSHARED_EXPORT QString staggerIndexToString(Map::StaggerIndex staggerIndex);
TILEDSHARED_EXPORT Map::StaggerIndex staggerIndexFromString(const QString &);

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

TILEDSHARED_EXPORT QString renderOrderToString(Map::RenderOrder renderOrder);
TILEDSHARED_EXPORT Map::RenderOrder renderOrderFromString(const QString &);

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Map::Orientation)
Q_DECLARE_METATYPE(Tiled::Map::LayerDataFormat)
Q_DECLARE_METATYPE(Tiled::Map::RenderOrder)

#endif // MAP_H
