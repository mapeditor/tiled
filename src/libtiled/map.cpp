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
#include "objecttemplate.h"
#include "tile.h"
#include "tilelayer.h"
#include "mapobject.h"

#include <QtMath>

using namespace Tiled;

Map::Map():
    Map(Orthogonal, 0, 0, 0, 0, false)
{
}

Map::Map(Orientation orientation,
         int width, int height, int tileWidth, int tileHeight, bool infinite):
    Object(MapType),
    mOrientation(orientation),
    mRenderOrder(RightDown),
    mCompressionLevel(-1),
    mWidth(width),
    mHeight(height),
    mTileWidth(tileWidth),
    mTileHeight(tileHeight),
    mInfinite(infinite),
    mHexSideLength(0),
    mStaggerAxis(StaggerY),
    mStaggerIndex(StaggerOdd),
    mChunkSize(CHUNK_SIZE, CHUNK_SIZE),
    mDrawMarginsDirty(true),
    mLayerDataFormat(Base64Zlib),
    mNextLayerId(1),
    mNextObjectId(1)
{
}

Map::Map(Orientation orientation,
         QSize size, QSize tileSize, bool infinite)
    : Map(orientation,
          size.width(), size.height(),
          tileSize.width(), tileSize.height(),
          infinite)
{
}

Map::~Map()
{
    qDeleteAll(mLayers);
}

void Map::setWidth(int width)
{
    if (width == mWidth)
        return;

    mWidth = width;
    emit widthChanged();
    emit sizeChanged();
}

void Map::setHeight(int height)
{
    if (height == mHeight)
        return;

    mHeight = height;
    emit heightChanged();
    emit sizeChanged();
}

void Map::setTileWidth(int width)
{
    if (width == mTileWidth)
        return;

    mTileWidth = width;
    emit tileWidthChanged();
}

void Map::setTileHeight(int height)
{
    if (height == mTileHeight)
        return;

    mTileHeight = height;
    emit tileHeightChanged();
}

QMargins Map::drawMargins() const
{
    if (mDrawMarginsDirty)
        recomputeDrawMargins();

    return mDrawMargins;
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

    for (const Layer *layer : allLayers()) {
        if (layer->isGroupLayer())
            continue;

        const QPointF offset = layer->totalOffset();
        offsetMargins = maxMargins(QMargins(qCeil(-offset.x()),
                                            qCeil(-offset.y()),
                                            qCeil(offset.x()),
                                            qCeil(offset.y())),
                                   offsetMargins);
    }

    return offsetMargins;
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

    mDrawMarginsDirty = false;
}

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

int Map::indexOfLayer(const QString &layerName, int layerTypes) const
{
    for (int index = 0; index < mLayers.size(); index++)
        if (layerAt(index)->name() == layerName
                && (layerTypes & layerAt(index)->layerType()))
            return index;

    return -1;
}

Layer *Map::findLayer(const QString &name, int layerTypes) const
{
    LayerIterator it(this, layerTypes);
    while (Layer *layer = it.next())
        if (layer->name() == name)
            return layer;
    return nullptr;
}

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

Layer *Map::takeLayerAt(int index)
{
    Layer *layer = mLayers.takeAt(index);
    layer->setMap(nullptr);
    return layer;
}

bool Map::addTileset(const SharedTileset &tileset)
{
    if (mTilesets.contains(tileset))
        return false;

    mTilesets.append(tileset);
    invalidateDrawMargins();
    return true;
}

void Map::addTilesets(const QSet<SharedTileset> &tilesets)
{
    for (const SharedTileset &tileset : tilesets)
        addTileset(tileset);
}

void Map::insertTileset(int index, const SharedTileset &tileset)
{
    Q_ASSERT(!mTilesets.contains(tileset));
    mTilesets.insert(index, tileset);
    invalidateDrawMargins();
}

int Map::indexOfTileset(const SharedTileset &tileset) const
{
    return mTilesets.indexOf(tileset);
}

void Map::removeTilesetAt(int index)
{
    mTilesets.remove(index);
    invalidateDrawMargins();
}

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

QSet<SharedTileset> Map::usedTilesets() const
{
    QSet<SharedTileset> tilesets;

    // Only top-level layers need to be considered, since GroupLayer goes over
    // its children
    for (const Layer *layer : mLayers)
        tilesets |= layer->usedTilesets();

    return tilesets;
}

bool Map::isTilesetUsed(const Tileset *tileset) const
{
    for (const Layer *layer : mLayers)
        if (layer->referencesTileset(tileset))
            return true;

    return false;
}

std::unique_ptr<Map> Map::clone() const
{
    auto o = std::make_unique<Map>(mOrientation, mWidth, mHeight, mTileWidth, mTileHeight, mInfinite);
    o->fileName = fileName;
    o->exportFileName = exportFileName;
    o->exportFormat = exportFormat;
    o->mRenderOrder = mRenderOrder;
    o->mHexSideLength = mHexSideLength;
    o->mStaggerAxis = mStaggerAxis;
    o->mStaggerIndex = mStaggerIndex;
    o->mBackgroundColor = mBackgroundColor;
    o->mChunkSize = mChunkSize;
    o->mDrawMargins = mDrawMargins;
    o->mDrawMarginsDirty = mDrawMarginsDirty;
    for (const Layer *layer : mLayers) {
        Layer *clone = layer->clone();
        clone->setMap(o.get());
        o->mLayers.append(clone);
    }
    o->mTilesets = mTilesets;
    o->mLayerDataFormat = mLayerDataFormat;
    o->mNextLayerId = mNextLayerId;
    o->mNextObjectId = mNextObjectId;
    o->setProperties(properties());
    return o;
}

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

QRegion Map::tileRegion() const
{
    QRegion region;
    LayerIterator it(this, Layer::TileLayerType);
    while (auto tileLayer = static_cast<TileLayer*>(it.next()))
        region |= tileLayer->region();
    return region;
}

QString Tiled::staggerAxisToString(Map::StaggerAxis staggerAxis)
{
    switch (staggerAxis) {
    case Map::StaggerY:
        return QLatin1String("y");
    case Map::StaggerX:
        return QLatin1String("x");
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
        return QLatin1String("odd");
    case Map::StaggerEven:
        return QLatin1String("even");
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
        return QLatin1String("unknown");
    case Map::Orthogonal:
        return QLatin1String("orthogonal");
    case Map::Isometric:
        return QLatin1String("isometric");
    case Map::Staggered:
        return QLatin1String("staggered");
    case Map::Hexagonal:
        return QLatin1String("hexagonal");
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
        return QLatin1String("gzip");
    case Map::Base64Zlib:
        return QLatin1String("zlib");
    case Map::Base64Zstandard:
        return QLatin1String("zstd");
    }
    return QString();
}

QString Tiled::renderOrderToString(Map::RenderOrder renderOrder)
{
    switch (renderOrder) {
    case Map::RightDown:
        return QLatin1String("right-down");
    case Map::RightUp:
        return QLatin1String("right-up");
    case Map::LeftDown:
        return QLatin1String("left-down");
    case Map::LeftUp:
        return QLatin1String("left-up");
    }
    return QString();
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
