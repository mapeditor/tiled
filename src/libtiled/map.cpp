/*
 * map.cpp
 * Copyright 2008-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2008, Roderic Morris <roderic@ccs.neu.edu>
 * Copyright 2010, Andrew G. Crowell <overkill9999@gmail.com>
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

#include "map.h"

#include "layer.h"
#include "tile.h"
#include "tilelayer.h"
#include "tileset.h"

using namespace Tiled;

Map::Map(Orientation orientation,
         int width, int height, int tileWidth, int tileHeight):
    mOrientation(orientation),
    mWidth(width),
    mHeight(height),
    mTileWidth(tileWidth),
    mTileHeight(tileHeight),
    mMaxTileSize(tileWidth, tileHeight)
{
}

Map::~Map()
{
    qDeleteAll(mLayers);
}

void Map::adjustMaxTileSize(const QSize &size)
{
    if (size.width() > mMaxTileSize.width())
        mMaxTileSize.setWidth(size.width());
    if (size.height() > mMaxTileSize.height())
        mMaxTileSize.setHeight(size.height());
}

int Map::tileLayerCount() const
{
    int count = 0;
    foreach (Layer *layer, mLayers)
       if (layer->asTileLayer())
           count++;
    return count;
}

int Map::objectLayerCount() const
{
    int count = 0;
    foreach (Layer *layer, mLayers)
        if (layer->asObjectGroup())
           count++;
    return count;
}

void Map::addLayer(Layer *layer)
{
    adoptLayer(layer);
    mLayers.append(layer);
}

int Map::indexOfLayer(const QString &layerName) const
{
    for (int index = 0; index < mLayers.size(); index++)
        if (layerAt(index)->name() == layerName)
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

    if (TileLayer *tileLayer = dynamic_cast<TileLayer*>(layer))
        adjustMaxTileSize(tileLayer->maxTileSize());
}

Layer *Map::takeLayerAt(int index)
{
    Layer *layer = mLayers.takeAt(index);
    layer->setMap(0);
    return layer;
}

void Map::addTileset(Tileset *tileset)
{
    mTilesets.append(tileset);
}

void Map::insertTileset(int index, Tileset *tileset)
{
    mTilesets.insert(index, tileset);
}

int Map::indexOfTileset(Tileset *tileset) const
{
    return mTilesets.indexOf(tileset);
}

void Map::removeTilesetAt(int index)
{
    mTilesets.removeAt(index);
}

void Map::replaceTileset(Tileset *oldTileset, Tileset *newTileset)
{
    const int index = mTilesets.indexOf(oldTileset);
    Q_ASSERT(index != -1);

    foreach (Layer *layer, mLayers)
        if (TileLayer *tileLayer = layer->asTileLayer())
            tileLayer->replaceReferencesToTileset(oldTileset, newTileset);

    mTilesets.removeAt(index);
    mTilesets.insert(index, newTileset);
}

bool Map::isTilesetUsed(Tileset *tileset) const
{
    foreach (const Layer *layer, mLayers)
        if (const TileLayer *tileLayer = dynamic_cast<const TileLayer*>(layer))
            if (tileLayer->referencesTileset(tileset))
                return true;

    return false;
}

Map *Map::clone() const
{
    Map *o = new Map(mOrientation, mWidth, mHeight, mTileWidth, mTileHeight);
    o->mMaxTileSize = mMaxTileSize;
    foreach (Layer *layer, mLayers)
        o->addLayer(layer->clone());
    o->mTilesets = mTilesets;
    o->setProperties(properties());
    return o;
}
