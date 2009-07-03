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

#ifndef TILESET_H
#define TILESET_H

#include <QList>
#include <QString>

namespace Tiled {

class Tile;

/**
 * A tileset, representing a set of tiles.
 */
class Tileset
{
public:
    /**
     * Constructor.
     *
     * @param name        the name of the tileset
     * @param tileWidth   the width of the tiles in the tileset
     * @param tileHeight  the height of the tiles in the tileset
     * @param tileSpacing the spacing between the tiles in the tileset image
     * @param margin      the margin around the tiles in the tileset image
     */
    Tileset(const QString &name, int tileWidth, int tileHeight,
            int tileSpacing = 0, int margin = 0):
        mName(name),
        mTileWidth(tileWidth),
        mTileHeight(tileHeight),
        mTileSpacing(tileSpacing),
        mMargin(margin)
    {
    }

    /**
     * Destructor.
     */
    ~Tileset();

    /**
     * Returns the name of this tileset.
     */
    const QString &name() const { return mName; }

    /**
     * Sets the name of this tileset.
     */
    void setName(const QString &name) { mName = name; }

    /**
     * Returns the file name of this tileset. When the tileset isn't an
     * external tileset, the file name is empty.
     */
    const QString &fileName() const { return mFileName; }

    /**
     * Sets the filename of this tileset.
     */
    void setFileName(const QString &fileName) { mFileName = fileName; }

    /**
     * Returns the width of the tiles in this tileset.
     */
    int tileWidth() const { return mTileWidth; }

    /**
     * Returns the height of the tiles in this tileset.
     */
    int tileHeight() const { return mTileHeight; }

    /**
     * Returns the spacing between the tiles in the tileset image.
     */
    int tileSpacing() const { return mTileSpacing; }

    /**
     * Returns the margin around the tiles in the tileset image.
     */
    int margin() const { return mMargin; }

    /**
     * Returns the tile for the given tile ID.
     */
    Tile *tileAt(int id) const;

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
     *                 as the image source of this tileset.
     * @return <code>true</code> if loading was succesful, otherwise
     *         returns <code>false</code>
     */
    bool loadFromImage(const QString &fileName);

    /**
     * Returns the file name of the external image that contains the tiles in
     * this tileset. Is an empty string when this tileset doesn't have a
     * tileset image.
     */
    const QString &imageSource() const { return mImageSource; }

private:
    QString mName;
    QString mFileName;
    QString mImageSource;
    int mTileWidth;
    int mTileHeight;
    int mTileSpacing;
    int mMargin;
    QList<Tile*> mTiles;
};

} // namespace Tiled

#endif // TILESET_H
