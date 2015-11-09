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

#include "layer.h"
#include "objectgroup.h"
#include "tile.h"
#include "tilelayer.h"
#include "mapobject.h"

#include <cmath>

using namespace Tiled;

Map::Map(Orientation orientation,
         int width, int height, int tileWidth, int tileHeight):
    Object(MapType),
    mOrientation(orientation),
    mRenderOrder(RightDown),
    mWidth(width),
    mHeight(height),
    mTileWidth(tileWidth),
    mTileHeight(tileHeight),
    mHexSideLength(0),
    mStaggerAxis(StaggerY),
    mStaggerIndex(StaggerOdd),
    mLayerDataFormat(Base64Zlib),
    mNextObjectId(1)
{
}

Map::Map(const Map &map):
    Object(map),
    mOrientation(map.mOrientation),
    mRenderOrder(map.mRenderOrder),
    mWidth(map.mWidth),
    mHeight(map.mHeight),
    mTileWidth(map.mTileWidth),
    mTileHeight(map.mTileHeight),
    mHexSideLength(map.mHexSideLength),
    mStaggerAxis(map.mStaggerAxis),
    mStaggerIndex(map.mStaggerIndex),
    mBackgroundColor(map.mBackgroundColor),
    mDrawMargins(map.mDrawMargins),
    mTilesets(map.mTilesets),
    mLayerDataFormat(map.mLayerDataFormat),
    mNextObjectId(1)
{
    for (const Layer *layer : map.mLayers) {
        Layer *clone = layer->clone();
        clone->setMap(this);
        mLayers.append(clone);
    }
}

Map::~Map()
{
    qDeleteAll(mLayers);
}

static QMargins maxMargins(const QMargins &a,
                           const QMargins &b)
{
    return QMargins(qMax(a.left(), b.left()),
                    qMax(a.top(), b.top()),
                    qMax(a.right(), b.right()),
                    qMax(a.bottom(), b.bottom()));
}

/**
 * Computes the extra margins due to layer offsets. These need to be taken into
 * account when determining the bounding rect of the map for example.
 */
QMargins Map::computeLayerOffsetMargins() const
{
    QMargins offsetMargins;

    for (const Layer *layer : mLayers) {
        const QPointF offset = layer->offset();
        offsetMargins = maxMargins(QMargins(std::ceil(-offset.x()),
                                            std::ceil(-offset.y()),
                                            std::ceil(offset.x()),
                                            std::ceil(offset.y())),
                                   offsetMargins);
    }

    return offsetMargins;
}

/**
 * Recomputes the draw margins for this map and each of its tilesets. Needed
 * after the tile offset of a tileset has changed for example.
 */
void Map::recomputeDrawMargins()
{
    int maxTileSize = 0;
    QMargins offsetMargins;

    for (const SharedTileset &tileset : mTilesets) {
        const QPoint offset = tileset->tileOffset();
        const QSize tileSize = tileset->tileSize();

        maxTileSize = std::max(maxTileSize, std::max(tileSize.width(),
                                                     tileSize.height()));

        offsetMargins = maxMargins(QMargins(-offset.x(),
                                            -offset.y(),
                                            offset.x(),
                                            offset.y()),
                                   offsetMargins);
    }

    // We subtract the tile size of the map, since that part does not
    // contribute to additional margin.
    mDrawMargins = QMargins(offsetMargins.left(),
                            offsetMargins.top() + maxTileSize - mTileHeight,
                            offsetMargins.right() + maxTileSize - mTileWidth,
                            offsetMargins.bottom());
}

int Map::layerCount(Layer::TypeFlag type) const
{
    int count = 0;
    for (Layer *layer : mLayers)
       if (layer->layerType() == type)
           count++;
    return count;
}

QList<Layer*> Map::layers(Layer::TypeFlag type) const
{
    QList<Layer*> layers;
    for (Layer *layer : mLayers)
        if (layer->layerType() == type)
            layers.append(layer);
    return layers;
}

QList<ObjectGroup*> Map::objectGroups() const
{
    QList<ObjectGroup*> layers;
    for (Layer *layer : mLayers)
        if (ObjectGroup *og = layer->asObjectGroup())
            layers.append(og);
    return layers;
}

QList<TileLayer*> Map::tileLayers() const
{
    QList<TileLayer*> layers;
    for (Layer *layer : mLayers)
        if (TileLayer *tl = layer->asTileLayer())
            layers.append(tl);
    return layers;
}

void Map::addLayer(Layer *layer)
{
    adoptLayer(layer);
    mLayers.append(layer);
}

int Map::indexOfLayer(const QString &layerName, unsigned layertypes) const
{
    for (int index = 0; index < mLayers.size(); index++)
        if (layerAt(index)->name() == layerName
                && (layertypes & layerAt(index)->layerType()))
            return index;

    return -1;
}

void Map::insertLayer(int index, Layer *layer)
{
    adoptLayer(layer);
    mLayers.insert(index, layer);
}

void Map::adoptLayer(Layer *layer)
{
    layer->setMap(this);

    if (ObjectGroup *group = layer->asObjectGroup()) {
        for (MapObject *o : group->objects()) {
            if (o->id() == 0)
                o->setId(takeNextObjectId());
        }
    }
}

Layer *Map::takeLayerAt(int index)
{
    Layer *layer = mLayers.takeAt(index);
    layer->setMap(nullptr);
    return layer;
}

void Map::addTileset(const SharedTileset &tileset)
{
    mTilesets.append(tileset);
}

void Map::addTilesets(const QSet<SharedTileset> &tilesets)
{
    for (const SharedTileset &tileset : tilesets)
        addTileset(tileset);
}

void Map::insertTileset(int index, const SharedTileset &tileset)
{
    mTilesets.insert(index, tileset);
}

int Map::indexOfTileset(const SharedTileset &tileset) const
{
    return mTilesets.indexOf(tileset);
}

void Map::removeTilesetAt(int index)
{
    mTilesets.remove(index);
}

void Map::replaceTileset(const SharedTileset &oldTileset,
                         const SharedTileset &newTileset)
{
    const int index = mTilesets.indexOf(oldTileset);
    Q_ASSERT(index != -1);

    for (Layer *layer : mLayers)
        layer->replaceReferencesToTileset(oldTileset.data(),
                                          newTileset.data());

    mTilesets.replace(index, newTileset);
}

bool Map::isTilesetUsed(const Tileset *tileset) const
{
    for (const Layer *layer : mLayers)
        if (layer->referencesTileset(tileset))
            return true;

    return false;
}


QString Tiled::staggerAxisToString(Map::StaggerAxis staggerAxis)
{
    switch (staggerAxis) {
    default:
    case Map::StaggerY:
        return QLatin1String("y");
        break;
    case Map::StaggerX:
        return QLatin1String("x");
        break;
    }
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
    default:
    case Map::StaggerOdd:
        return QLatin1String("odd");
        break;
    case Map::StaggerEven:
        return QLatin1String("even");
        break;
    }
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
    default:
    case Map::Unknown:
        return QLatin1String("unknown");
        break;
    case Map::Orthogonal:
        return QLatin1String("orthogonal");
        break;
    case Map::Isometric:
        return QLatin1String("isometric");
        break;
    case Map::Staggered:
        return QLatin1String("staggered");
        break;
    case Map::Hexagonal:
        return QLatin1String("hexagonal");
        break;
    }
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

QString Tiled::renderOrderToString(Map::RenderOrder renderOrder)
{
    switch (renderOrder) {
    default:
    case Map::RightDown:
        return QLatin1String("right-down");
        break;
    case Map::RightUp:
        return QLatin1String("right-up");
        break;
    case Map::LeftDown:
        return QLatin1String("left-down");
        break;
    case Map::LeftUp:
        return QLatin1String("left-up");
        break;
    }
}

Map::RenderOrder Tiled::renderOrderFromString(const QString &string)
{
    Map::RenderOrder renderOrder = Map::RightDown;
    if (string == QLatin1String("right-up")) {
        renderOrder = Map::RightUp;
    } else if (string == QLatin1String("left-down")) {
        renderOrder = Map::LeftDown;
    } else if (string == QLatin1String("left-up")) {
        renderOrder = Map::LeftUp;
    }
    return renderOrder;
}

Map *Map::fromLayer(Layer *layer)
{
    Map *result = new Map(Unknown, layer->width(), layer->height(), 0, 0);
    result->addLayer(layer);
    return result;
}
