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

#pragma once

#include "layer.h"
#include "object.h"
#include "tileset.h"

#include <QColor>
#include <QList>
#include <QMargins>
#include <QSharedPointer>
#include <QSize>
#include <QVector>

#include <memory>

namespace Tiled {

class MapObject;
class ObjectGroup;
class ObjectTemplate;
class Tile;

/**
 * A tile map. Consists of a stack of layers.
 *
 * It also keeps track of the list of referenced tilesets.
 */
class TILEDSHARED_EXPORT Map : public Object
{
    Q_OBJECT

    Q_PROPERTY(int width READ width NOTIFY widthChanged)
    Q_PROPERTY(int height READ height NOTIFY heightChanged)
    Q_PROPERTY(int tileWidth READ tileWidth NOTIFY tileWidthChanged)
    Q_PROPERTY(int tileHeight READ tileHeight NOTIFY tileHeightChanged)
    Q_PROPERTY(QSize size READ size NOTIFY sizeChanged)

    class LayerIteratorHelper
    {
    public:
        LayerIteratorHelper(const Map &map, int layerTypes);

        LayerIterator begin() const;
        LayerIterator end() const;
        bool isEmpty() const;

    private:
        const Map &mMap;
        const int mLayerTypes;
    };

public:
    QString fileName;
    QString exportFileName;
    QString exportFormat;

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
    Q_ENUM(Orientation)

    /**
     * The different formats in which the tile layer data can be stored.
     */
    enum LayerDataFormat {
        XML             = 0,
        Base64          = 1,
        Base64Gzip      = 2,
        Base64Zlib      = 3,
        Base64Zstandard = 4,
        CSV             = 5
    };
    Q_ENUM(LayerDataFormat)

    /**
     * The order in which tiles are rendered on screen.
     */
    enum RenderOrder {
        RightDown  = 0,
        RightUp    = 1,
        LeftDown   = 2,
        LeftUp     = 3
    };
    Q_ENUM(RenderOrder)

    /**
     * Which axis is staggered. Only used by the isometric staggered and
     * hexagonal map renderers.
     */
    enum StaggerAxis {
        StaggerX,
        StaggerY
    };
    Q_ENUM(StaggerAxis)

    /**
     * When staggering, specifies whether the odd or the even rows/columns are
     * shifted half a tile right/down. Only used by the isometric staggered and
     * hexagonal map renderers.
     */
    enum StaggerIndex {
        StaggerOdd  = 0,
        StaggerEven = 1
    };
    Q_ENUM(StaggerIndex)

    Map();

    /**
     * Constructor taking map orientation, size and tile size as parameters.
     */
    Map(Orientation orientation,
        int width, int height,
        int tileWidth, int tileHeight,
        bool infinite = false);

    Map(Orientation orientation,
        QSize size,
        QSize tileSize,
        bool infinite = false);

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
     * Returns the compression level of this map.
     */
    int compressionLevel() const { return mCompressionLevel; }

    /**
     * Sets the compression level of this map.
     */
    void setCompressionLevel(int compressionLevel) { mCompressionLevel = compressionLevel; }

    /**
     * Returns the width of this map in tiles.
     */
    int width() const { return mWidth; }

    /**
     * Sets the width of this map in tiles.
     */
    void setWidth(int width);

    /**
     * Returns the height of this map in tiles.
     */
    int height() const { return mHeight; }

    /**
     * Sets the height of this map in tiles.
     */
    void setHeight(int height);

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
    void setTileWidth(int width);

    /**
     * Returns the tile height used by this map.
     */
    int tileHeight() const { return mTileHeight; }

    /**
     * Sets the height of one tile.
     */
    void setTileHeight(int height);

    bool infinite() const { return mInfinite; }

    void setInfinite(bool infinite) { mInfinite = infinite; }

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
    void invertStaggerIndex();

    /**
     * Returns the margins that have to be taken into account when figuring
     * out which part of the map to repaint after changing some tiles.
     */
    QMargins drawMargins() const;
    void invalidateDrawMargins();

    QMargins computeLayerOffsetMargins() const;

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

    int groupLayerCount() const
    { return layerCount(Layer::GroupLayerType); }

    /**
     * Returns the top-level layer at the specified \a index.
     */
    Layer *layerAt(int index) const
    { return mLayers.at(index); }

    /**
     * Returns the list of top-level layers of this map.
     */
    const QList<Layer*> &layers() const { return mLayers; }

    LayerIteratorHelper allLayers(int layerTypes = Layer::AnyLayerType) const;
    LayerIteratorHelper tileLayers() const;
    LayerIteratorHelper objectGroups() const;

    /**
     * Adds a layer to this map.
     */
    void addLayer(std::unique_ptr<Layer> layer);
    void addLayer(Layer *layer);

    /**
     * Returns the index of the layer given by \a layerName, or -1 if no
     * layer with that name is found.
     *
     * The second optional parameter specifies the layer types which are
     * searched.
     *
     * @deprecated Does not support group layers. Use findLayer() instead.
     */
    int indexOfLayer(const QString &layerName,
                     int layerTypes = Layer::AnyLayerType) const;

    /**
     * Returns the first layer with the given \a name, or nullptr if no
     * layer with that name is found.
     *
     * The second optional parameter specifies the layer types which are
     * searched.
     */
    Layer *findLayer(const QString &name,
                     int layerTypes = Layer::AnyLayerType) const;

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
     * @return whether the tileset wasn't already part of the map
     */
    bool addTileset(const SharedTileset &tileset);

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
     *
     * @return whether the new tileset was added to the map
     */
    bool replaceTileset(const SharedTileset &oldTileset,
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
     * Returns the tilesets that have been added to this map.
     */
    const QVector<SharedTileset> &tilesets() const { return mTilesets; }

    /**
     * Computes the tilesets that are used by this map.
     */
    QSet<SharedTileset> usedTilesets() const;

    /**
     * Returns a list of MapObjects to be updated in the map scene
     */
    QList<MapObject*> replaceObjectTemplate(const ObjectTemplate *oldObjectTemplate,
                                            const ObjectTemplate *newObjectTemplate);

    /**
     * Returns the background color of this map.
     */
    const QColor &backgroundColor() const { return mBackgroundColor; }

    /**
     * Sets the background color of this map.
     */
    void setBackgroundColor(QColor color) { mBackgroundColor = color; }

    /**
     * Returns the chunk size used when saving tile layers of this map.
     */
    QSize chunkSize() const { return mChunkSize; }
    
    /**
     * Sets the chunk size used when saving tile layers of this map.
     */
    void setChunkSize(QSize size) { mChunkSize = size; }
    
    /**
     * Returns whether the given \a tileset is used by any tile layer of this
     * map.
     */
    bool isTilesetUsed(const Tileset *tileset) const;

    std::unique_ptr<Map> clone() const;

    /**
     * Returns whether the map is staggered
     */
    bool isStaggered() const
    { return orientation() == Hexagonal || orientation() == Staggered; }

    LayerDataFormat layerDataFormat() const
    { return mLayerDataFormat; }
    void setLayerDataFormat(LayerDataFormat format)
    { mLayerDataFormat = format; }

    void setNextLayerId(int nextId);
    int nextLayerId() const;
    int takeNextLayerId();

    void setNextObjectId(int nextId);
    int nextObjectId() const;
    int takeNextObjectId();
    void initializeObjectIds(ObjectGroup &objectGroup);

    Layer *findLayerById(int layerId) const;
    MapObject *findObjectById(int objectId) const;

    QRegion tileRegion() const;

signals:
    void widthChanged();
    void heightChanged();
    void tileWidthChanged();
    void tileHeightChanged();
    void sizeChanged();

private:
    friend class GroupLayer;    // so it can call adoptLayer

    void adoptLayer(Layer &layer);

    void recomputeDrawMargins() const;

    Orientation mOrientation;
    RenderOrder mRenderOrder;
    int mCompressionLevel;
    int mWidth;
    int mHeight;
    int mTileWidth;
    int mTileHeight;
    bool mInfinite;
    int mHexSideLength;
    StaggerAxis mStaggerAxis;
    StaggerIndex mStaggerIndex;
    QColor mBackgroundColor;
    QSize mChunkSize;
    mutable QMargins mDrawMargins;
    mutable bool mDrawMarginsDirty;
    QList<Layer*> mLayers;
    QVector<SharedTileset> mTilesets;
    LayerDataFormat mLayerDataFormat;
    int mNextLayerId;
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

inline void Map::invertStaggerIndex()
{
    mStaggerIndex = static_cast<StaggerIndex>(!mStaggerIndex);
}

inline void Map::invalidateDrawMargins()
{
    mDrawMarginsDirty = true;
}

/**
 * Returns a helper for iterating all tile layers of the given \a layerTypes
 * in this map.
 */
inline Map::LayerIteratorHelper Map::allLayers(int layerTypes) const
{
    return LayerIteratorHelper { *this, layerTypes };
}

/**
 * Returns a helper for iterating all tile layers in this map.
 */
inline Map::LayerIteratorHelper Map::tileLayers() const
{
    return allLayers(Layer::TileLayerType);
}

/**
 * Returns a helper for iterating all object groups in this map.
 */
inline Map::LayerIteratorHelper Map::objectGroups() const
{
    return allLayers(Layer::ObjectGroupType);
}

inline void Map::addLayer(std::unique_ptr<Layer> layer)
{
    addLayer(layer.release());
}

/**
 * Sets the next id to be used for layers of this map.
 */
inline void Map::setNextLayerId(int nextId)
{
    Q_ASSERT(nextId > 0);
    mNextLayerId = nextId;
}

/**
 * Returns the next layer id for this map.
 */
inline int Map::nextLayerId() const
{
    return mNextLayerId;
}

/**
 * Returns the next layer id for this map and allocates a new one.
 */
inline int Map::takeNextLayerId()
{
    return mNextLayerId++;
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


inline Map::LayerIteratorHelper::LayerIteratorHelper(const Map &map, int layerTypes)
    : mMap(map)
    , mLayerTypes(layerTypes)
{}

inline LayerIterator Map::LayerIteratorHelper::begin() const
{
    LayerIterator iterator(&mMap, mLayerTypes);
    iterator.next();
    return iterator;
}

inline LayerIterator Map::LayerIteratorHelper::end() const
{
    LayerIterator iterator(&mMap, mLayerTypes);
    iterator.toBack();
    return iterator;
}

inline bool Map::LayerIteratorHelper::isEmpty() const
{
    return LayerIterator(&mMap, mLayerTypes).next() == nullptr;
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

/**
 * Helper function that returns a string representing the compression used by
 * the given layer data format.
 *
 * @return The compression as a lowercase string.
 */
TILEDSHARED_EXPORT QString compressionToString(Map::LayerDataFormat);

TILEDSHARED_EXPORT QString renderOrderToString(Map::RenderOrder renderOrder);
TILEDSHARED_EXPORT Map::RenderOrder renderOrderFromString(const QString &);

typedef QSharedPointer<Map> SharedMap;

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Map::Orientation)
Q_DECLARE_METATYPE(Tiled::Map::LayerDataFormat)
Q_DECLARE_METATYPE(Tiled::Map::RenderOrder)
