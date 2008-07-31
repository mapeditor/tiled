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

#ifndef TILESET_H
#define TILESET_H

#include <QPixmap>
#include <QList>
#include <QString>

namespace Tiled {

/**
 * A tileset, representing a set of tiles.
 */
class Tileset
{
    public:
        /**
         * Constructor.
         */
        Tileset(const QString &name, int tileWidth, int tileHeight,
                int spacing = 0):
            mName(name),
            mTileWidth(tileWidth),
            mTileHeight(tileHeight),
            mSpacing(spacing)
        {
        }

        /**
         * Returns the name of this tileset.
         */
        const QString &name() const { return mName; }

        /**
         * Sets the name of this tileset.
         */
        void setName(const QString &name) { mName = name; }

        /**
         * Returns the image for the given tile ID.
         */
        QPixmap tileImageAt(int id) const;

        /**
         * Returns the number of tiles in this tileset.
         */
        int tileCount() const { return mTiles.size(); }

        /**
         * Load this tileset from the given tileset image. This will cause any
         * existing tiles in this tileset to be thrown out.
         *
         * The tile width and height of this tileset must be higher than 0.
         *
         * @param fileName the file name of the image, which will be remembered
         *                 as the source of this tileset.
         * @return <code>true</code> if loading was succesful, otherwise
         *         returns <code>false</code>
         */
        bool loadFromImage(const QString &fileName);

    private:
        QString mName;
        QString mSource;
        int mTileWidth;
        int mTileHeight;
        int mSpacing;
        QList<QPixmap> mTiles;
};

} // namespace Tiled

#endif // TILESET_H
