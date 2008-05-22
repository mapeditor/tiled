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

#ifndef LAYER_H
#define LAYER_H

#include <QImage>
#include <QString>
#include <QVector>

namespace Tiled {

class Map;

/**
 * A map layer.
 */
class Layer {
    public:
        /**
         * Constructor.
         */
        Layer(const QString &name, int x, int y, int width, int height,
              Map *map = 0);

        /**
         * Destructor.
         */
        ~Layer() {}

        /**
         * Returns the name of this layer.
         */
        const QString &name() const { return mName; }

        /**
         * Sets the name of this layer.
         */
        void setName(const QString &name) { mName = name; }

        /**
         * Returns the map this layer is part of.
         */
        Map *map() const { return mMap; }

        /**
         * Sets the map this layer is part of.
         */
        void setMap(Map *map) { mMap = map; }

        /**
         * Returns the x position of this layer (in tiles).
         */
        int x() const { return mX; }

        /**
         * Returns the y position of this layer (in tiles).
         */
        int y() const { return mY; }

        /**
         * Returns the width of this layer.
         */
        int width() const { return mWidth; }

        /**
         * Returns the height of this layer.
         */
        int height() const { return mHeight; }

        /**
         * Returns the maximum tile height of this layer. Used by the layer
         * rendering code to determine the area that needs to be redrawn.
         */
        int maxTileHeight() const { return mMaxTileHeight; }

        /**
         * Returns the tile at the given coordinates. The coordinates have to
         * be within this layer.
         */
        const QImage& tileAt(int x, int y) const;

        /**
         * Sets the tile for the given coordinates.
         */
        void setTile(int x, int y, const QImage &img);

    private:
        QString mName;
        int mX;
        int mY;
        int mWidth;
        int mHeight;
        int mMaxTileHeight;
        Map *mMap;
        QVector<QImage> mTiles;
};

} // namespace Tiled

#endif // LAYER_H
