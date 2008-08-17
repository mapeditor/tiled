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

#ifndef TILE_H
#define TILE_H

#include <QMap>
#include <QPixmap>
#include <QString>

namespace Tiled {

class Tileset;

class Tile
{
public:
    Tile(const QPixmap &image, int id, Tileset *tileset):
        mId(id),
        mTileset(tileset),
        mImage(image)
    {}

    /**
     * Returns ID of this tile within its tileset.
     */
    int id() const { return mId; }

    /**
     * Returns the tileset that this tile is part of.
     */
    Tileset *tileset() const { return mTileset; }

    /**
     * Returns the image of this tile.
     */
    const QPixmap &image() const { return mImage; }

    /**
     * Returns the width of this tile.
     */
    int width() const { return mImage.width(); }

    /**
     * Returns the height of this tile.
     */
    int height() const { return mImage.height(); }

    /**
     * Returns a pointer to the properties of this tile. This allows
     * modification of the properties.
     */
    QMap<QString, QString>* properties() { return &mProperties; }

    /**
     * Returns a copy of the properties of this map.
     */
    QMap<QString, QString> properties() const { return mProperties; }

private:
    int mId;
    Tileset *mTileset;
    QPixmap mImage;
    QMap<QString, QString> mProperties;
};

} // namespace Tiled

#endif // TILE_H
