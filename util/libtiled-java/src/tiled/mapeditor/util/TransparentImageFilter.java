/*
 *  Tiled Map Editor, (c) 2004-2006
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Adam Turk <aturk@biggeruniverse.com>
 *  Bjorn Lindeijer <bjorn@lindeijer.nl>
 */

package tiled.mapeditor.util;

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
