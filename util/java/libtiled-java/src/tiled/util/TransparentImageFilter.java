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

import java.awt.image.RGBImageFilter;

/**
 * This filter is used for filtering out a given "transparent" color from an
 * image. Sometimes known as magic pink.
 */
public class TransparentImageFilter extends RGBImageFilter
{
    int trans;

    /**
     * @param col the color to make transparent
     */
    public TransparentImageFilter(int col) {
        trans = col;

        // The filter doesn't depend on pixel location
        canFilterIndexColorModel = true;
    }

    /**
     * Filters the given pixel. It returns a transparent pixel for pixels that
     * match the transparency color, or the existing pixel for anything else.
     */
    public int filterRGB(int x, int y, int rgb) {
        if (rgb == trans) {
            return 0;
        } else {
            return rgb;
        }
    }
}
