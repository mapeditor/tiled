/*
 * Tiled Map Editor (Qt)
 * Copyright 2008 Tiled (Qt) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt).
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
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307, USA.
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

void Map::addLayer(Layer *layer)
{
    adoptLayer(layer);
    mLayers.append(layer);
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

void Map::removeTileset(Tileset *tileset)
{
    mTilesets.removeOne(tileset);
}

QList<Tileset*> Map::tilesets() const
{
    return mTilesets;
}

Map *Map::clone() const
{
    Map *o = new Map(mOrientation, mWidth, mHeight, mTileWidth, mTileHeight);
    o->mMaxTileSize = mMaxTileSize;
    foreach (Layer *layer, mLayers)
        o->addLayer(layer->clone());
    o->mTilesets = mTilesets;
    o->mProperties = mProperties;
    return o;
}
