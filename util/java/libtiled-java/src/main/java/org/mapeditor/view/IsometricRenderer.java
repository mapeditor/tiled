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
package org.mapeditor.view;

import java.awt.Dimension;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.image.BufferedImage;

import org.mapeditor.core.Map;
import org.mapeditor.core.ObjectGroup;
import org.mapeditor.core.Tile;
import org.mapeditor.core.TileLayer;

/**
 * The isometric map renderer.
 *
 * @author Thorbjørn Lindeijer
 * @author Adam Turk
 * @author Mike Thomas
 * @version 1.0.2
 */
public class IsometricRenderer implements MapRenderer {

    private final Map map;

    /**
     * <p>Constructor for IsometricRenderer.</p>
     *
     * @param map a {@link org.mapeditor.core.Map} object.
     */
    public IsometricRenderer(Map map) {
        this.map = map;
    }

    /** {@inheritDoc} */
    @Override
    public Dimension getMapSize() {
        int side = map.getHeight() + map.getWidth();
        return new Dimension(
            side * map.getTileWidth() / 2,
            side * map.getTileHeight() / 2);
    }

    /** {@inheritDoc} */
    @Override
    public void paintTileLayer(Graphics2D g, TileLayer layer) {
        final Rectangle clip = g.getClipBounds();
        final int tileWidth = map.getTileWidth();
        final int tileHeight = map.getTileHeight();

        // Translate origin to top-center
        double tileRatio = (double) tileWidth / (double) tileHeight;
        clip.x -= map.getHeight() * (tileWidth / 2);
        int mx = clip.y + (int) (clip.x / tileRatio);
        int my = clip.y - (int) (clip.x / tileRatio);

        // Calculate map coords and divide by tile size (tiles assumed to
        // be square in normal projection)
        Point rowItr = new Point(
                (mx < 0 ? mx - tileHeight : mx) / tileHeight,
                (my < 0 ? my - tileHeight : my) / tileHeight);
        rowItr.x--;

        // Location on the screen of the top corner of a tile.
        int originX = (map.getHeight() * tileWidth) / 2;
        Point drawLoc = new Point(
                ((rowItr.x - rowItr.y) * tileWidth / 2) + originX,
                (rowItr.x + rowItr.y) * tileHeight / 2);
        drawLoc.x -= tileWidth / 2;
        drawLoc.y -= tileHeight / 2;

        // Add offset from tile layer property
        drawLoc.x += layer.getOffsetX() != null ? layer.getOffsetX() : 0;
        drawLoc.y += layer.getOffsetY() != null ? layer.getOffsetY() : 0;
        
        // Determine area to draw from clipping rectangle
        int tileStepY = tileHeight / 2 == 0 ? 1 : tileHeight / 2;
        int columns = clip.width / tileWidth + 3;
        int rows = clip.height / tileStepY + 4;

        // Draw this map layer
        for (int y = 0; y < rows; y++) {
            Point columnItr = new Point(rowItr);

            for (int x = 0; x < columns; x++) {
                final Tile tile = layer.getTileAt(columnItr.x, columnItr.y);

                if (tile != null) {
                    final BufferedImage image = tile.getImage();
                    if (image == null) {
                        continue;
                    }

                    // Add offset from tileset property
                    drawLoc.x += tile.getTileSet().getTileoffset() != null ? tile.getTileSet().getTileoffset().getX() : 0;
                    drawLoc.y += tile.getTileSet().getTileoffset() != null ? tile.getTileSet().getTileoffset().getY() : 0;

                    g.drawImage(image, drawLoc.x, drawLoc.y, null);
                }

                // Advance to the next tile
                columnItr.x++;
                columnItr.y--;
                drawLoc.x += tileWidth;
            }

            // Advance to the next row
            if ((y & 1) > 0) {
                rowItr.x++;
                drawLoc.x += tileWidth / 2;
            } else {
                rowItr.y++;
                drawLoc.x -= tileWidth / 2;
            }
            drawLoc.x -= columns * tileWidth;
            drawLoc.y += tileStepY;
        }
    }

    /** {@inheritDoc} */
    @Override
    public void paintObjectGroup(Graphics2D g, ObjectGroup group) {
        throw new UnsupportedOperationException("Not supported yet.");
    }
}
