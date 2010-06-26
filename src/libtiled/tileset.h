/*
 * tileset.h
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyrigth 2009, Edward Hutchins <eah1@yahoo.com>
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

#ifndef TILESET_H
#define TILESET_H

#include "tiled_global.h"

#include <QColor>
#include <QList>
#include <QString>

namespace Tiled {

class Tile;

/**
 * A tileset, representing a set of tiles.
 *
 * This class currently only supports loading tiles from a tileset image, using
 * loadFromImage(). There is no way to add or remove arbitrary tiles.
 */
class TILEDSHARED_EXPORT Tileset
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
        mMargin(margin),
        mColumnCount(0)
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
     * Returns whether this tileset is external.
     */
    bool isExternal() const { return !mFileName.isEmpty(); }

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
     * Returns the number of tile columns in the tileset image.
     */
    int columnCount() const { return mColumnCount; }

    /**
     * Returns the transparent color, or an invalid color if no transparent
     * color is used.
     */
    QColor transparentColor() const { return mTransparentColor; }

    /**
     * Sets the transparent color. Pixels with this color will be masked out
     * when loadFromImage() is called.
     */
    void setTransparentColor(const QColor &c) { mTransparentColor = c; }

    /**
     * Load this tileset from the given tileset image. This will replace
     * existing tile images in this tileset with new ones. If the new image
     * contains more tiles than exist in the tileset new tiles will be
     * appended, if there are fewer tiles the excess images will be blanked.
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
    QColor mTransparentColor;
    int mTileWidth;
    int mTileHeight;
    int mTileSpacing;
    int mMargin;
    int mColumnCount;
    QList<Tile*> mTiles;
};

} // namespace Tiled

#endif // TILESET_H
