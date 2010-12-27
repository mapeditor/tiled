/*
 * Copyright 2004-2006, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2004-2006, Adam Turk <aturk@biggeruniverse.com>
 *
 * This file is part of libtiled-java.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library;  If not, see <http://www.gnu.org/licenses/>.
 */

package tiled.util;

import java.awt.Dimension;
import java.awt.Image;
import java.awt.image.BufferedImage;

/**
 * A generic interface to a class that implements tile cutting behavior.
 */
public interface TileCutter
{
    /**
     * Sets the image that this cutter should cut in tile images.
     * @param image the image that this cutter should cut
     */
    public void setImage(BufferedImage image);

    /**
     * Retrieves the next tile image.
     * @return the next tile image, or <code>null</code> when no more tile
     *         images are available
     */
    public Image getNextTile();

    /**
     * Resets the tile cutter so that the next call to <code>getNextTile</code>
     * will return the first tile.
     */
    void reset();

    /**
     * Returns the default tile dimensions of tiles cut by this cutter.
     * @return the default tile dimensions of tiles cut by this cutter.
     */
    public Dimension getTileDimensions();

    /**
     * Returns the name of this tile cutter.
     * @return the name of this tile cutter
     */
    public String getName();
}
