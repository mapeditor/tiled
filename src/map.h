/*
 * Tiled Map Editor (Qt port)
 * Copyright 2008 Tiled (Qt port) developers (see AUTHORS file)
 *
 * This file is part of Tiled (Qt port).
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

#ifndef MAP_H
#define MAP_H

#include <QList>
#include <QMap>
#include <QPixmap>
#include <QString>

namespace Tiled {

class Layer;
class Tileset;

/**
 * A tile map.
 */
class Map {
    public:
        /**
         * Constructor, taking map and tile size as parameters.
         */
        Map(int width, int height, int tileWidth, int tileHeight);

        /**
         * Destructor.
         */
        ~Map();

        /**
         * Returns the width of this map.
         */
        int width() const { return mWidth; }

        /**
         * Sets the width of this map.
         */
        void setWidth(int width) { mWidth = width; }

        /**
         * Returns the height of this map.
         */
        int height() const { return mHeight; }

        /**
         * Sets the height of this map.
         */
        void setHeight(int height) { mHeight = height; }

        /**
         * Returns the tile width of this map.
         */
        int tileWidth() const { return mTileWidth; }

        /**
         * Returns the tile height used by this map.
         */
        int tileHeight() const { return mTileHeight; }

        /**
         * Returns the list of layers of this map.
         */
        const QList<Layer*>& layers() const { return mLayers; }

        /**
         * Adds a layer to this map.
         */
        void addLayer(Layer *layer);

        /**
         * Adds a layer to this map, inserting it at the given index.
         */
        void insertLayer(int index, Layer *layer);

        /**
         * Returns a pointer to the properties of this map.
         */
        QMap<QString, QString>* properties() { return &mProperties; }

        /**
         * Adds a tileset to this map.
         *
         * @param tileset the tileset to add
         * @param firstGid the map-global ID of the first tile in the tileset
         */
        void addTileset(Tileset *tileset, int firstGid);

        /**
         * Returns the tile for the given global tile ID.
         *
         * @param gid the global tile ID, must be at least 0
         */
        QPixmap tileForGid(int gid) const;

    private:
        int mWidth;
        int mHeight;
        int mTileWidth;
        int mTileHeight;
        int mMaxTileHeight;
        QList<Layer*> mLayers;
        QMap<int, Tileset*> mTilesets;
        QMap<QString, QString> mProperties;
};

} // namespace Tiled

#endif // MAP_H
