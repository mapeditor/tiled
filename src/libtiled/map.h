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
    class LayerIteratorHelper
    {
    public:
        LayerIteratorHelper(const Map &map, int layerTypes);

        LayerIterator begin() const;
        LayerIterator end() const;
        bool isEmpty() const;
        QList<Layer*> toList() const;

    private:
        const Map &mMap;
        const int mLayerTypes;
    };

public:
    QString fileName;
    QString exportFileName;
    QString exportFormat;

    enum Property {
        TileWidthProperty,
        TileHeightProperty,
        InfiniteProperty,
        HexSideLengthProperty,
        StaggerAxisProperty,
        StaggerIndexProperty,
        ParallaxOriginProperty,
        OrientationProperty,
        RenderOrderProperty,
        BackgroundColorProperty,
        LayerDataFormatProperty,
        CompressionLevelProperty,
        ChunkSizeProperty
    };

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
        XML             = 0,
        Base64          = 1,
        Base64Gzip      = 2,
        Base64Zlib      = 3,
        Base64Zstandard = 4,
        CSV             = 5
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

    struct Parameters
    {
        Orientation orientation = Orthogonal;
        RenderOrder renderOrder = RightDown;
        int width = 0;
        int height = 0;
        int tileWidth = 0;
        int tileHeight = 0;
        bool infinite = false;
        int hexSideLength = 0;
        StaggerAxis staggerAxis = StaggerY;
        StaggerIndex staggerIndex = StaggerOdd;
        QPointF parallaxOrigin;
        QColor backgroundColor;
    };

    struct EditorSettings
    {
        int compressionLevel = -1;
        QSize chunkSize = QSize(CHUNK_SIZE, CHUNK_SIZE);
        LayerDataFormat layerDataFormat = Base64Zlib;
    };

    Map();
    Map(const Parameters &parameters);

    /**
     * Constructor taking map orientation, size and tile size as parameters.
     *
     * @deprecated Only kept around for the Python API!
     */
    Map(Orientation orientation,
        int width, int height,
        int tileWidth, int tileHeight);

    ~Map() override;

    const Parameters &parameters() const;

    Orientation orientation() const;
    void setOrientation(Orientation orientation);

    RenderOrder renderOrder() const;
    void setRenderOrder(RenderOrder renderOrder);

    int compressionLevel() const;
    void setCompressionLevel(int compressionLevel);

    int width() const;
    void setWidth(int width);

    int height() const;
    void setHeight(int height);

    QSize size() const;

    int tileWidth() const;
    void setTileWidth(int width);

    int tileHeight() const;
    void setTileHeight(int height);

    QSize tileSize() const;

    bool infinite() const;
    void setInfinite(bool infinite);

    int hexSideLength() const;
    void setHexSideLength(int hexSideLength);

    StaggerAxis staggerAxis() const;
    void setStaggerAxis(StaggerAxis staggerAxis);

    StaggerIndex staggerIndex() const;
    void setStaggerIndex(StaggerIndex staggerIndex);
    void invertStaggerIndex();

    QPointF parallaxOrigin() const;
    void setParallaxOrigin(const QPointF &parallaxOrigin);

    QMargins drawMargins() const;
    void invalidateDrawMargins();

    void adjustBoundingRectForOffsetsAndImageLayers(QRect &boundingRect) const;

    int layerCount() const;
    int layerCount(Layer::TypeFlag type) const;

    int tileLayerCount() const;
    int objectGroupCount() const;
    int imageLayerCount() const;
    int groupLayerCount() const;

    Layer *layerAt(int index) const;

    const QList<Layer*> &layers() const;

    LayerIteratorHelper allLayers(int layerTypes = Layer::AnyLayerType) const;
    LayerIteratorHelper tileLayers() const;
    LayerIteratorHelper objectGroups() const;

    void addLayer(std::unique_ptr<Layer> layer);
    void addLayer(Layer *layer);

    Layer *findLayer(const QString &name,
                     int layerTypes = Layer::AnyLayerType) const;

    void insertLayer(int index, Layer *layer);

    Layer *takeLayerAt(int index);

    bool addTileset(const SharedTileset &tileset);
    void addTilesets(const QSet<SharedTileset> &tilesets);
    void insertTileset(int index, const SharedTileset &tileset);

    int indexOfTileset(const SharedTileset &tileset) const;

    void removeTilesetAt(int index);

    bool replaceTileset(const SharedTileset &oldTileset,
                        const SharedTileset &newTileset);

    int tilesetCount() const;
    SharedTileset tilesetAt(int index) const;
    const QVector<SharedTileset> &tilesets() const;

    QSet<SharedTileset> usedTilesets() const;

    QList<MapObject*> replaceObjectTemplate(const ObjectTemplate *oldObjectTemplate,
                                            const ObjectTemplate *newObjectTemplate);

    const QColor &backgroundColor() const;
    void setBackgroundColor(QColor color);

    QSize chunkSize() const;
    void setChunkSize(QSize size);
    
    bool isTilesetUsed(const Tileset *tileset) const;

    std::unique_ptr<Map> clone() const;

    void copyLayers(const QList<Layer*> &layers,
                    const QRegion &tileRegion,
                    Map &targetMap) const;

    void normalizeTileLayerPositionsAndMapSize();

    bool isStaggered() const;

    LayerDataFormat layerDataFormat() const;
    void setLayerDataFormat(LayerDataFormat format);

    void setNextLayerId(int nextId);
    int nextLayerId() const;
    int takeNextLayerId();

    void setNextObjectId(int nextId);
    int nextObjectId() const;
    int takeNextObjectId();
    void initializeObjectIds(ObjectGroup &objectGroup);

    Layer *findLayerById(int layerId) const;
    MapObject *findObjectById(int objectId) const;

    QRect tileBoundingRect() const;
    QRegion modifiedTileRegion() const;

private:
    friend class GroupLayer;    // so it can call adoptLayer

    void adoptLayer(Layer &layer);

    void recomputeDrawMargins() const;

    Parameters mParameters;
    EditorSettings mEditorSettings;

    mutable QMargins mDrawMargins;
    mutable bool mDrawMarginsDirty = true;

    QList<Layer*> mLayers;
    QVector<SharedTileset> mTilesets;

    int mNextLayerId = 1;
    int mNextObjectId = 1;
};


inline const Map::Parameters &Map::parameters() const
{
    return mParameters;
}

inline Map::Orientation Map::orientation() const
{
    return mParameters.orientation;
}

inline void Map::setOrientation(Map::Orientation orientation)
{
    mParameters.orientation = orientation;
}

inline Map::RenderOrder Map::renderOrder() const
{
    return mParameters.renderOrder;
}

inline void Map::setRenderOrder(Map::RenderOrder renderOrder)
{
    mParameters.renderOrder = renderOrder;
}

/**
 * Returns the compression level used for compressed tile layer data.
 */
inline int Map::compressionLevel() const
{
    return mEditorSettings.compressionLevel;
}

inline void Map::setCompressionLevel(int compressionLevel)
{
    mEditorSettings.compressionLevel = compressionLevel;
}

/**
 * Returns the width of this map in tiles.
 */
inline int Map::width() const
{
    return mParameters.width;
}

/**
 * Sets the width of this map in tiles.
 */
inline void Map::setWidth(int width)
{
    mParameters.width = width;
}

/**
 * Returns the height of this map in tiles.
 */
inline int Map::height() const
{
    return mParameters.height;
}

/**
 * Sets the height of this map in tiles.
 */
inline void Map::setHeight(int height)
{
    mParameters.height = height;
}

/**
 * Returns the size of this map. Provided for convenience.
 */
inline QSize Map::size() const
{
    return QSize(mParameters.width, mParameters.height);
}

/**
 * Returns the tile width of this map.
 */
inline int Map::tileWidth() const
{
    return mParameters.tileWidth;
}

/**
 * Sets the width of one tile.
 */
inline void Map::setTileWidth(int width)
{
    mParameters.tileWidth = width;
}

/**
 * Returns the tile height used by this map.
 */
inline int Map::tileHeight() const
{
    return mParameters.tileHeight;
}

/**
 * Sets the height of one tile.
 */
inline void Map::setTileHeight(int height)
{
    mParameters.tileHeight = height;
}

/**
 * Returns the size of one tile. Provided for convenience.
 */
inline QSize Map::tileSize() const
{
    return QSize(mParameters.tileWidth, mParameters.tileHeight);
}

inline bool Map::infinite() const
{
    return mParameters.infinite;
}

inline void Map::setInfinite(bool infinite)
{
    mParameters.infinite = infinite;
}

inline int Map::hexSideLength() const
{
    return mParameters.hexSideLength;
}

inline void Map::setHexSideLength(int hexSideLength)
{
    mParameters.hexSideLength = hexSideLength;
}

inline Map::StaggerAxis Map::staggerAxis() const
{
    return mParameters.staggerAxis;
}

inline void Map::setStaggerAxis(StaggerAxis staggerAxis)
{
    mParameters.staggerAxis = staggerAxis;
}

inline Map::StaggerIndex Map::staggerIndex() const
{
    return mParameters.staggerIndex;
}

inline void Map::setStaggerIndex(StaggerIndex staggerIndex)
{
    mParameters.staggerIndex = staggerIndex;
}

inline void Map::invertStaggerIndex()
{
    mParameters.staggerIndex = static_cast<StaggerIndex>(!mParameters.staggerIndex);
}

inline QPointF Map::parallaxOrigin() const
{
    return mParameters.parallaxOrigin;
}

inline void Map::setParallaxOrigin(const QPointF &parallaxOrigin)
{
    mParameters.parallaxOrigin = parallaxOrigin;
}

inline void Map::invalidateDrawMargins()
{
    mDrawMarginsDirty = true;
}

/**
 * Returns the number of layers of this map.
 */
inline int Map::layerCount() const
{
    return mLayers.size();
}

inline int Map::tileLayerCount() const
{
    return layerCount(Layer::TileLayerType);
}

inline int Map::objectGroupCount() const
{
    return layerCount(Layer::ObjectGroupType);
}

inline int Map::imageLayerCount() const
{
    return layerCount(Layer::ImageLayerType);
}

inline int Map::groupLayerCount() const
{
    return layerCount(Layer::GroupLayerType);
}

/**
 * Returns the top-level layer at the specified \a index.
 */
inline Layer *Map::layerAt(int index) const
{
    return mLayers.at(index);
}

/**
 * Returns the list of top-level layers of this map.
 */
inline const QList<Layer *> &Map::layers() const
{
    return mLayers;
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

/**
 * Adds a layer to this map.
 */
inline void Map::addLayer(std::unique_ptr<Layer> layer)
{
    addLayer(layer.release());
}

/**
 * Returns the number of tilesets of this map.
 */
inline int Map::tilesetCount() const
{
    return mTilesets.size();
}

/**
 * Returns the tileset at the given index.
 */
inline SharedTileset Map::tilesetAt(int index) const
{
    return mTilesets.at(index);
}

/**
 * Returns the tilesets that have been added to this map.
 */
inline const QVector<SharedTileset> &Map::tilesets() const
{
    return mTilesets;
}

/**
 * Returns the background color of this map.
 */
inline const QColor &Map::backgroundColor() const
{
    return mParameters.backgroundColor;
}

inline void Map::setBackgroundColor(QColor color)
{
    mParameters.backgroundColor = color;
}

/**
 * Returns the chunk size used when saving tile layers of this map.
 */
inline QSize Map::chunkSize() const
{
    return mEditorSettings.chunkSize;
}

inline void Map::setChunkSize(QSize size)
{
    mEditorSettings.chunkSize = size;
}

/**
 * Returns whether the map is staggered.
 */
inline bool Map::isStaggered() const
{
    return orientation() == Hexagonal || orientation() == Staggered;
}

inline Map::LayerDataFormat Map::layerDataFormat() const
{
    return mEditorSettings.layerDataFormat;
}

inline void Map::setLayerDataFormat(Map::LayerDataFormat format)
{
    mEditorSettings.layerDataFormat = format;
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

inline QList<Layer *> Map::LayerIteratorHelper::toList() const
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return QList<Layer *>(begin(), end());
#else
    LayerIterator iterator(&mMap, mLayerTypes);
    QList<Layer *> layers;
    while (Layer *layer = iterator.next())
        layers.append(layer);
    return layers;
#endif
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

using SharedMap = QSharedPointer<Map>;

} // namespace Tiled

Q_DECLARE_METATYPE(Tiled::Map*)
Q_DECLARE_METATYPE(Tiled::Map::Orientation)
Q_DECLARE_METATYPE(Tiled::Map::LayerDataFormat)
Q_DECLARE_METATYPE(Tiled::Map::RenderOrder)
