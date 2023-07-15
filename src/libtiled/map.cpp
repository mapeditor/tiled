/*
 * map.cpp
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

#include "map.h"

#include "imagelayer.h"
#include "layer.h"
#include "mapobject.h"
#include "objectgroup.h"
#include "objecttemplate.h"
#include "tilelayer.h"

#include <QtMath>

using namespace Tiled;

Map::Map()
    : Map(Parameters())
{
}

Map::Map(const Parameters &parameters)
    : Object(MapType)
    , mParameters(parameters)
{
}

Map::Map(Orientation orientation,
         int width, int height,
         int tileWidth, int tileHeight)
    : Map()
{
    mParameters.orientation = orientation;
    mParameters.width = width;
    mParameters.height = height;
    mParameters.tileWidth = tileWidth;
    mParameters.tileHeight = tileHeight;
}

Map::~Map()
{
    qDeleteAll(mLayers);
}

/**
 * Returns the margins that have to be taken into account when figuring
 * out which part of the map to repaint after changing some tiles.
 */
QMargins Map::drawMargins() const
{
    if (mDrawMarginsDirty)
        recomputeDrawMargins();

    return mDrawMargins;
}

/**
 * Adjusts the given \a boundingRect to account for layer offsets and image
 * layers.
 *
 * The bounding rect is assumed to already cover the tile content of the map in
 * pixels. This function may extend it, when an offset has been applied to any
 * tile layers and when any image layers extend beyond the map.
 */
void Map::adjustBoundingRectForOffsetsAndImageLayers(QRect &boundingRect) const
{
    QMargins offsetMargins;

    for (const Layer *layer : allLayers()) {
        switch (layer->layerType()) {
        case Layer::TileLayerType: {
            // Offset tile layers currently always contribute to the bounding
            // rect. More precise would be to take into account their
            // contents.
            const QPointF offset = layer->totalOffset();
            offsetMargins = maxMargins(QMargins(qCeil(-offset.x()),
                                                qCeil(-offset.y()),
                                                qCeil(offset.x()),
                                                qCeil(offset.y())),
                                       offsetMargins);
            break;
        }
        case Layer::ImageLayerType: {
            auto imageLayer = static_cast<const ImageLayer*>(layer);
            const QRect bounds = QRectF(layer->totalOffset(),
                                        imageLayer->image().size()).toAlignedRect();
            if (!imageLayer->repeatX()) {
                boundingRect.setRight(qMax(bounds.right(), boundingRect.right()));
                boundingRect.setLeft(qMin(bounds.left(), boundingRect.left()));
            }
            if (!imageLayer->repeatY()) {
                boundingRect.setTop(qMin(bounds.top(), boundingRect.top()));
                boundingRect.setBottom(qMax(bounds.bottom(), boundingRect.bottom()));
            }
            break;
        }
        case Layer::ObjectGroupType:
            // Objects are entirely ignored when determining which part of a
            // map to render.
            break;
        case Layer::GroupLayerType:
            break;
        }
    }

    boundingRect += offsetMargins;
}

/**
 * Recomputes the draw margins for this map and each of its tilesets. Needed
 * after the tile offset of a tileset has changed for example.
 */
void Map::recomputeDrawMargins() const
{
    int maxTileSize = 0;
    QMargins offsetMargins;

    for (const SharedTileset &tileset : mTilesets) {
        const bool useGridSize = tileset->tileRenderSize() == Tileset::GridSize;
        const QSize tileSize = useGridSize ? this->tileSize()
                                           : tileset->tileSize();

        maxTileSize = std::max(maxTileSize, std::max(tileSize.width(),
                                                     tileSize.height()));

        const QPoint offset = tileset->tileOffset();
        offsetMargins = maxMargins(QMargins(-offset.x(),
                                            -offset.y(),
                                            offset.x(),
                                            offset.y()),
                                   offsetMargins);
    }

    // We subtract the tile size of the map, since that part does not
    // contribute to additional margin.
    mDrawMargins = QMargins(offsetMargins.left(),
                            offsetMargins.top() + maxTileSize - tileHeight(),
                            offsetMargins.right() + maxTileSize - tileWidth(),
                            offsetMargins.bottom());

    mDrawMarginsDirty = false;
}

/**
 * Convenience function that returns the number of layers of this map that
 * match the given \a type.
 */
int Map::layerCount(Layer::TypeFlag type) const
{
    int count = 0;
    LayerIterator iterator(this, type);
    while (iterator.next())
       count++;
    return count;
}

void Map::addLayer(Layer *layer)
{
    adoptLayer(*layer);
    mLayers.append(layer);
}

/**
 * Returns the first layer with the given \a name, or nullptr if no
 * layer with that name is found.
 *
 * The second optional parameter specifies the layer types which are
 * searched.
 */
Layer *Map::findLayer(const QString &name, int layerTypes) const
{
    LayerIterator it(this, layerTypes);
    while (Layer *layer = it.next())
        if (layer->name() == name)
            return layer;
    return nullptr;
}

/**
 * Adds a layer to this map, inserting it at the given index.
 */
void Map::insertLayer(int index, Layer *layer)
{
    adoptLayer(*layer);
    mLayers.insert(index, layer);
}

void Map::adoptLayer(Layer &layer)
{
    if (layer.id() == 0)
        layer.setId(takeNextLayerId());

    layer.setMap(this);

    if (ObjectGroup *group = layer.asObjectGroup())
        initializeObjectIds(*group);
}

/**
 * Removes the layer at the given index from this map and returns it.
 * The caller becomes responsible for the lifetime of this layer.
 */
Layer *Map::takeLayerAt(int index)
{
    Layer *layer = mLayers.takeAt(index);
    layer->setMap(nullptr);
    return layer;
}

/**
 * Adds a tileset to this map. The map does not take ownership over its
 * tilesets, this is merely for keeping track of which tilesets are used by
 * the map, and their saving order.
 *
 * @param tileset the tileset to add
 * @return whether the tileset wasn't already part of the map
 */
bool Map::addTileset(const SharedTileset &tileset)
{
    if (mTilesets.contains(tileset))
        return false;

    mTilesets.append(tileset);
    invalidateDrawMargins();
    return true;
}

/**
 * Convenience function to be used together with Layer::usedTilesets()
 */
void Map::addTilesets(const QSet<SharedTileset> &tilesets)
{
    for (const SharedTileset &tileset : tilesets)
        addTileset(tileset);
}

/**
 * Inserts \a tileset at \a index in the list of tilesets used by this map.
 */
void Map::insertTileset(int index, const SharedTileset &tileset)
{
    Q_ASSERT(!mTilesets.contains(tileset));
    mTilesets.insert(index, tileset);
    invalidateDrawMargins();
}

/**
 * Returns the index of the given \a tileset, or -1 if it is not used in
 * this map.
 */
int Map::indexOfTileset(const SharedTileset &tileset) const
{
    return mTilesets.indexOf(tileset);
}

/**
 * Removes the tileset at \a index from this map.
 *
 * \warning Does not make sure that this map no longer refers to tiles from
 *          the removed tileset!
 *
 * \sa addTileset
 */
void Map::removeTilesetAt(int index)
{
    mTilesets.remove(index);
    invalidateDrawMargins();
}

/**
 * Replaces all tiles from \a oldTileset with tiles from \a newTileset.
 * Also replaces the old tileset with the new tileset in the list of
 * tilesets.
 *
 * @return whether the new tileset was added to the map
 */
bool Map::replaceTileset(const SharedTileset &oldTileset,
                         const SharedTileset &newTileset)
{
    Q_ASSERT(oldTileset != newTileset);

    const int index = mTilesets.indexOf(oldTileset);
    Q_ASSERT(index != -1);

    const auto &layers = mLayers;
    for (Layer *layer : layers) {
        layer->replaceReferencesToTileset(oldTileset.data(),
                                          newTileset.data());
    }

    invalidateDrawMargins();

    if (mTilesets.contains(newTileset)) {
        mTilesets.remove(index);
        return false;
    } else {
        mTilesets.replace(index, newTileset);
        return true;
    }
}

/**
 * Computes the tilesets that are used by this map.
 */
QSet<SharedTileset> Map::usedTilesets() const
{
    QSet<SharedTileset> tilesets;

    // Only top-level layers need to be considered, since GroupLayer goes over
    // its children
    for (const Layer *layer : mLayers)
        tilesets |= layer->usedTilesets();

    return tilesets;
}

/**
 * Returns whether the given \a tileset is used by any tile layer of this
 * map.
 */
bool Map::isTilesetUsed(const Tileset *tileset) const
{
    for (const Layer *layer : mLayers)
        if (layer->referencesTileset(tileset))
            return true;

    return false;
}

std::unique_ptr<Map> Map::clone() const
{
    auto o = std::make_unique<Map>(mParameters);
    o->setClassName(className());
    o->setProperties(properties());
    o->fileName = fileName;
    o->exportFileName = exportFileName;
    o->exportFormat = exportFormat;
    o->mEditorSettings = mEditorSettings;
    o->mDrawMargins = mDrawMargins;
    o->mDrawMarginsDirty = mDrawMarginsDirty;
    for (const Layer *layer : mLayers) {
        Layer *clone = layer->clone();
        clone->setMap(o.get());
        o->mLayers.append(clone);
    }
    o->mTilesets = mTilesets;
    o->mNextLayerId = mNextLayerId;
    o->mNextObjectId = mNextObjectId;
    return o;
}

/**
 * Copies the given \a tileRegion of the \a layers to \a targetMap.
 *
 * Empty layers are included as well, to ensure a more consistent behavior when
 * copy/pasting multiple layers.
 *
 * Currently only copies tile layers.
 */
void Map::copyLayers(const QList<Layer *> &layers,
                     const QRegion &tileRegion,
                     Map &targetMap) const
{
    LayerIterator layerIterator(this);
    while (Layer *layer = layerIterator.next()) {
        switch (layer->layerType()) {
        case Layer::TileLayerType: {
            if (!layers.contains(layer))    // ignore unselected tile layers
                continue;

            const TileLayer *tileLayer = static_cast<const TileLayer*>(layer);
            const QRegion area = tileRegion.intersected(tileLayer->bounds());

            // Copy the selected part of the layer
            auto copyLayer = tileLayer->copy(area.translated(-tileLayer->position()));
            copyLayer->setId(tileLayer->id());
            copyLayer->setName(tileLayer->name());
            copyLayer->setPosition(area.boundingRect().topLeft());
            copyLayer->setOpacity(tileLayer->opacity());
            copyLayer->setTintColor(tileLayer->tintColor());

            targetMap.addLayer(std::move(copyLayer));
            break;
        }
        case Layer::ObjectGroupType: // todo: maybe it makes sense to group selected objects by layer
        case Layer::ImageLayerType:
        case Layer::GroupLayerType:
            break;  // nothing to do
        }
    }
}

/**
 * Determines the unified content area of all tile layers and then repositions
 * those layers to eliminate unnecessary offset. Also sets the size of the map
 * to encompass the final tile layer contents exactly.
 */
void Map::normalizeTileLayerPositionsAndMapSize()
{
    LayerIterator it(this, Layer::TileLayerType);

    QRect contentRect;
    while (auto tileLayer = static_cast<TileLayer*>(it.next()))
        contentRect |= tileLayer->region().boundingRect();

    if (!contentRect.topLeft().isNull()) {
        it.toFront();
        while (auto tileLayer = static_cast<TileLayer*>(it.next()))
            tileLayer->setPosition(tileLayer->position() - contentRect.topLeft());

        // Adjust the stagger index when layers are moved by odd amounts
        const int staggerOffset = (staggerAxis() == Map::StaggerX ? contentRect.x()
                                                                  : contentRect.y()) % 2;
        setStaggerIndex(static_cast<Map::StaggerIndex>((staggerIndex() + staggerOffset) % 2));
    }

    setWidth(contentRect.width());
    setHeight(contentRect.height());
}

/**
 * Returns a list of MapObjects to be updated in the map scene
 */
QList<MapObject*> Map::replaceObjectTemplate(const ObjectTemplate *oldObjectTemplate,
                                             const ObjectTemplate *newObjectTemplate)
{
    Q_ASSERT(oldObjectTemplate != newObjectTemplate);

    QList<MapObject*> changedObjects;

    for (auto layer : objectGroups()) {
        for (auto o : static_cast<ObjectGroup*>(layer)->objects()) {
            if (o->objectTemplate() == oldObjectTemplate) {
                o->setObjectTemplate(newObjectTemplate);
                o->syncWithTemplate();
                changedObjects.append(o);
            }
        }
    }

    return changedObjects;
}

void Map::initializeObjectIds(ObjectGroup &objectGroup)
{
    for (MapObject *o : objectGroup) {
        if (o->id() == 0)
            o->setId(takeNextObjectId());
    }
}

Layer *Map::findLayerById(int layerId) const
{
    for (Layer *layer : allLayers()) {
        if (layer->id() == layerId)
            return layer;
    }
    return nullptr;
}

MapObject *Map::findObjectById(int objectId) const
{
    for (Layer *layer : objectGroups()) {
        for (MapObject *mapObject : static_cast<ObjectGroup*>(layer)->objects()) {
            if (mapObject->id() == objectId)
                return mapObject;
        }
    }
    return nullptr;
}

/**
 * Returns the area occupied by tiles. Usually simply matches the map size,
 * but for infinite maps it returns a rough (chunks based) bounding rectangle
 * covering the contents of all tile layers.
 */
QRect Map::tileBoundingRect() const
{
    if (!infinite())
        return QRect(0, 0, width(), height());

    QRect mapBounds;

    LayerIterator iterator(this, Layer::TileLayerType);
    while (TileLayer *tileLayer = static_cast<TileLayer*>(iterator.next()))
        mapBounds = mapBounds.united(tileLayer->bounds());

    if (mapBounds.size() == QSize(0, 0))
        mapBounds.setSize(QSize(1, 1));

    return mapBounds;
}

QRegion Map::modifiedTileRegion() const
{
    QRegion region;
    LayerIterator it(this, Layer::TileLayerType);
    while (auto tileLayer = static_cast<TileLayer*>(it.next()))
        region |= tileLayer->modifiedRegion();
    return region;
}

QString Tiled::staggerAxisToString(Map::StaggerAxis staggerAxis)
{
    switch (staggerAxis) {
    case Map::StaggerY:
        return QStringLiteral("y");
    case Map::StaggerX:
        return QStringLiteral("x");
    }
    return QString();
}

Map::StaggerAxis Tiled::staggerAxisFromString(const QString &string)
{
    Map::StaggerAxis staggerAxis = Map::StaggerY;
    if (string == QLatin1String("x"))
        staggerAxis = Map::StaggerX;
    return staggerAxis;
}

QString Tiled::staggerIndexToString(Map::StaggerIndex staggerIndex)
{
    switch (staggerIndex) {
    case Map::StaggerOdd:
        return QStringLiteral("odd");
    case Map::StaggerEven:
        return QStringLiteral("even");
    }
    return QString();
}

Map::StaggerIndex Tiled::staggerIndexFromString(const QString &string)
{
    Map::StaggerIndex staggerIndex = Map::StaggerOdd;
    if (string == QLatin1String("even"))
        staggerIndex = Map::StaggerEven;
    return staggerIndex;
}

QString Tiled::orientationToString(Map::Orientation orientation)
{
    switch (orientation) {
    case Map::Unknown:
        return QStringLiteral("unknown");
    case Map::Orthogonal:
        return QStringLiteral("orthogonal");
    case Map::Isometric:
        return QStringLiteral("isometric");
    case Map::Staggered:
        return QStringLiteral("staggered");
    case Map::Hexagonal:
        return QStringLiteral("hexagonal");
    }
    return QString();
}

Map::Orientation Tiled::orientationFromString(const QString &string)
{
    Map::Orientation orientation = Map::Unknown;
    if (string == QLatin1String("orthogonal")) {
        orientation = Map::Orthogonal;
    } else if (string == QLatin1String("isometric")) {
        orientation = Map::Isometric;
    } else if (string == QLatin1String("staggered")) {
        orientation = Map::Staggered;
    } else if (string == QLatin1String("hexagonal")) {
        orientation = Map::Hexagonal;
    }
    return orientation;
}

QString Tiled::compressionToString(Map::LayerDataFormat layerDataFormat)
{
    switch (layerDataFormat) {
    case Map::XML:
    case Map::Base64:
    case Map::CSV:
        return QString();
    case Map::Base64Gzip:
        return QStringLiteral("gzip");
    case Map::Base64Zlib:
        return QStringLiteral("zlib");
    case Map::Base64Zstandard:
        return QStringLiteral("zstd");
    }
    return QString();
}

QString Tiled::renderOrderToString(Map::RenderOrder renderOrder)
{
    switch (renderOrder) {
    case Map::RightDown:
        return QStringLiteral("right-down");
    case Map::RightUp:
        return QStringLiteral("right-up");
    case Map::LeftDown:
        return QStringLiteral("left-down");
    case Map::LeftUp:
        return QStringLiteral("left-up");
    }
    return QString();
}

Map::RenderOrder Tiled::renderOrderFromString(const QString &string)
{
    Map::RenderOrder renderOrder = Map::RightDown;
    if (string == QLatin1String("right-up"))
        renderOrder = Map::RightUp;
    else if (string == QLatin1String("left-down"))
        renderOrder = Map::LeftDown;
    else if (string == QLatin1String("left-up"))
        renderOrder = Map::LeftUp;
    return renderOrder;
}
