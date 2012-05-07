/*
 * Copyright 2004-2006, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2004-2006, Adam Turk <aturk@biggeruniverse.com>
 *
 * This file is part of libtiled-java.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
