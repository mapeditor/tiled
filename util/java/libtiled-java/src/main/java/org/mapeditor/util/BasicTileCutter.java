/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2004 - 2017 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright (C) 2004 - 2017 Adam Turk <aturk@biggeruniverse.com>
 * Copyright (C) 2016 - 2017 Mike Thomas <mikepthomas@outlook.com>
 * %%
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 * #L%
 */
package org.mapeditor.util;

import java.awt.image.BufferedImage;

/**
 * Cuts tiles from a tileset image according to a regular rectangular pattern.
 * Supports a variable spacing between tiles and a margin around them.
 *
 * @author Thorbjørn Lindeijer
 * @author Adam Turk
 * @author Mike Thomas
 * @version 1.0.2
 */
public class BasicTileCutter implements TileCutter {

    private int nextX, nextY;
    private BufferedImage image;
    private final int tileWidth;
    private final int tileHeight;
    private final int tileSpacing;
    private final int tileMargin;

    /**
     * <p>Constructor for BasicTileCutter.</p>
     *
     * @param tileWidth a int.
     * @param tileHeight a int.
     * @param tileSpacing a int.
     * @param tileMargin a int.
     */
    public BasicTileCutter(int tileWidth, int tileHeight, int tileSpacing,
            int tileMargin) {
        this.tileWidth = tileWidth;
        this.tileHeight = tileHeight;
        this.tileSpacing = tileSpacing;
        this.tileMargin = tileMargin;

        reset();
    }

    /** {@inheritDoc} */
    @Override
    public String getName() {
        return "Basic";
    }

    /** {@inheritDoc} */
    @Override
    public void setImage(BufferedImage image) {
        this.image = image;
    }

    /** {@inheritDoc} */
    @Override
    public BufferedImage getNextTile() {
        if (nextY + tileHeight + tileMargin <= image.getHeight()) {
            BufferedImage tile
                    = image.getSubimage(nextX, nextY, tileWidth, tileHeight);
            nextX += tileWidth + tileSpacing;

            if (nextX + tileWidth + tileMargin > image.getWidth()) {
                nextX = tileMargin;
                nextY += tileHeight + tileSpacing;
            }

            return tile;
        }

        return null;
    }

    /** {@inheritDoc} */
    @Override
    public final void reset() {
        nextX = tileMargin;
        nextY = tileMargin;
    }

    /** {@inheritDoc} */
    @Override
    public int getTileWidth() {
        return tileWidth;
    }

    /** {@inheritDoc} */
    @Override
    public int getTileHeight() {
        return tileHeight;
    }

    /**
     * Returns the spacing between tile images.
     *
     * @return the spacing between tile images.
     */
    public int getTileSpacing() {
        return tileSpacing;
    }

    /**
     * Returns the margin around the tile images.
     *
     * @return the margin around the tile images.
     */
    public int getTileMargin() {
        return tileMargin;
    }

    /**
     * Returns the number of tiles per row in the tileset image.
     *
     * @return the number of tiles per row in the tileset image.
     */
    public int getColumns() {
        return (image.getWidth() - 2 * tileMargin + tileSpacing)
                / (tileWidth + tileSpacing);
    }
}
