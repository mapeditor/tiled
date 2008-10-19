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
#include "tileset.h"

using namespace Tiled;

Map::Map(int width, int height, int tileWidth, int tileHeight):
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

void Map::addLayer(Layer *layer)
{
    layer->setMap(this);
    mLayers.append(layer);
}

void Map::insertLayer(int index, Layer *layer)
{
    layer->setMap(this);
    mLayers.insert(index, layer);
}

Layer *Map::takeLayerAt(int index)
{
    Layer *layer = mLayers.takeAt(index);
    layer->setMap(0);
    return layer;
}

void Map::addTileset(Tileset *tileset, int firstGid)
{
    mTilesets.insert(firstGid, tileset);
}

QMap<int, Tileset*> Map::tilesets() const
{
    return mTilesets;
}

Tile* Map::tileForGid(int gid) const
{
    Q_ASSERT(gid >= 0);

    if (gid == 0)
        return 0;

    // TODO: This assertion can fail on trying to load an invalid map file
    //       should result in a readable error messages instead of a crash
    Q_ASSERT(mTilesets.size() > 0);

    // Find the tileset containing this tile
    QMap<int, Tileset*>::const_iterator i = mTilesets.upperBound(gid);
    --i; // Navigate one tileset back since upper bound finds the next
    const int tileId = gid - i.key();
    const Tileset *tileset = i.value();

    return (tileset) ? tileset->tileAt(tileId) : 0;
}

int Map::gidForTile(const Tile *tile) const
{
    if (!tile)
        return 0;

    const Tileset *tileset = tile->tileset();

    // Find the first GID for the tileset
    QMap<int, Tileset*>::const_iterator i = mTilesets.begin();
    QMap<int, Tileset*>::const_iterator i_end = mTilesets.end();
    while (i != i_end && i.value() != tileset)
        ++i;

    return (i != i_end) ? i.key() + tile->id() : 0;
}
