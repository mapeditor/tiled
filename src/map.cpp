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
    mMaxTileHeight(tileHeight)
{
}

Map::~Map()
{
    qDeleteAll(mLayers);
}

void Map::adjustMaxTileHeight(int height)
{
    if (height > mMaxTileHeight)
        mMaxTileHeight = height;
}

/**
 * Converts the given point from tile to pixel coordinates.
 */
QPoint Map::toPixelCoordinates(const QPointF &p) const
{
    return QPoint(qRound(p.x() * mTileWidth),
                  qRound(p.y() * mTileHeight));
}

/**
 * Converts the given size from tile to pixel coordinates.
 */
QSize Map::toPixelCoordinates(const QSizeF &p) const
{
    return QSize(qRound(p.width() * mTileWidth),
                 qRound(p.height() * mTileHeight));
}

/**
 * Converts the given point from pixel to tile coordinates.
 */
QPointF Map::toTileCoordinates(const QPoint &p) const
{
    return QPointF((qreal) p.x() / mTileWidth,
                   (qreal) p.y() / mTileHeight);
}

/**
 * Converts the given size from pixel to tile coordinates.
 */
QSizeF Map::toTileCoordinates(const QSize &p) const
{
    return QSizeF((qreal) p.width() / mTileWidth,
                  (qreal) p.height() / mTileHeight);
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
        adjustMaxTileHeight(tileLayer->maxTileHeight());
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

QList<Tileset*> Map::tilesets() const
{
    return mTilesets;
}

Map *Map::clone() const
{
    Map *o = new Map(mOrientation, mWidth, mHeight, mTileWidth, mTileHeight);
    o->mMaxTileHeight = mMaxTileHeight;
    foreach (Layer *layer, mLayers)
        o->addLayer(layer->clone());
    o->mTilesets = mTilesets;
    o->mProperties = mProperties;
    return o;
}
