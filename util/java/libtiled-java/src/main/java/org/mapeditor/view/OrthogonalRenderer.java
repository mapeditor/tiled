/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2004 - 2020 Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright (C) 2004 - 2020 Adam Turk <aturk@biggeruniverse.com>
 * Copyright (C) 2016 - 2020 Mike Thomas <mikepthomas@outlook.com>
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

import java.awt.Color;
import java.awt.Dimension;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.geom.AffineTransform;

import org.mapeditor.core.Map;
import org.mapeditor.core.MapObject;
import org.mapeditor.core.ObjectGroup;
import org.mapeditor.core.Tile;
import org.mapeditor.core.TileLayer;

/**
 * The orthogonal map renderer. This is the most basic map renderer, dealing
 * with maps that use rectangular tiles.
 *
 * @version 1.4.2
 */
public class OrthogonalRenderer extends AbstractRenderer {

    private final Map map;

    /**
     * Constructor for OrthogonalRenderer.
     *
     * @param map a {@link org.mapeditor.core.Map} object.
     */
    public OrthogonalRenderer(Map map) {
        this.map = map;
    }

    /** {@inheritDoc} */
    @Override
    public Dimension getMapSize() {
        if (map.getInfinite() != null && map.getInfinite() == 1) {
            int minX = 0, minY = 0;
            int maxX = map.getWidth(), maxY = map.getHeight();
            for (int i = 0; i < map.getLayerCount(); i++) {
                Rectangle b = map.getLayer(i).getBounds();
                minX = Math.min(minX, b.x);
                minY = Math.min(minY, b.y);
                maxX = Math.max(maxX, b.x + b.width);
                maxY = Math.max(maxY, b.y + b.height);
            }
            return new Dimension(
                    (maxX - minX) * map.getTileWidth(),
                    (maxY - minY) * map.getTileHeight());
        }
        return new Dimension(
                map.getWidth() * map.getTileWidth(),
                map.getHeight() * map.getTileHeight());
    }

    /** {@inheritDoc} */
    @Override
    public void paintTileLayer(Graphics2D g, TileLayer layer) {
        paintLayer(g, layer, () -> {
            final Rectangle clip = g.getClipBounds();
            final int tileWidth = map.getTileWidth();
            final int tileHeight = map.getTileHeight();
            final Rectangle bounds = layer.getBounds();

            // Compute visible tile range in tile-space coordinates
            final int startX = Math.max(bounds.x,
                    Math.floorDiv(clip.x, tileWidth));
            final int startY = Math.max(bounds.y,
                    Math.floorDiv(clip.y, tileHeight));
            final int endX = Math.min(bounds.x + bounds.width,
                    (int) Math.ceil(clip.getMaxX() / tileWidth));
            final int endY = Math.min(bounds.y + bounds.height,
                    (int) Math.ceil((clip.getMaxY() + map.getTileHeightMax()) / tileHeight));

            for (int tx = startX; tx < endX; ++tx) {
                for (int ty = startY; ty < endY; ++ty) {
                    final Tile tile = layer.getTileAt(tx, ty);
                    if (tile == null) {
                        continue;
                    }
                    final Image image = tile.getImage();
                    if (image == null) {
                        continue;
                    }

                    Point drawLoc = getTileDrawLocation(
                            layer, tile, tx * tileWidth, (ty + 1) * tileHeight - image.getHeight(null));

                    drawTileWithFlags(g, image, drawLoc.x, drawLoc.y, layer.getFlagsAt(tx, ty), false);
                }
            }
        });
    }

    /** {@inheritDoc} */
    @Override
    public void paintObjectGroup(Graphics2D g, ObjectGroup group) {
        paintLayer(g, group, () -> {
            final Dimension tsize = new Dimension(map.getTileWidth(), map.getTileHeight());
            assert tsize.width != 0 && tsize.height != 0;
            final Rectangle bounds = map.getBounds();

            g.translate(
                    bounds.x * tsize.width,
                    bounds.y * tsize.height);
            try {
                for (MapObject mo : group) {
                    final double ox = mo.getX();
                    final double oy = mo.getY();
                    final double rotation = mo.getRotation();
                    final Tile tile = mo.getTile();

                    if (tile != null) {
                        Image objectImage = tile.getImage();
                        AffineTransform old = g.getTransform();
                        g.rotate(Math.toRadians(rotation));
                        Point drawLoc = getTileDrawLocation(tile, (int) ox, (int) oy - objectImage.getHeight(null));
                        g.drawImage(objectImage, drawLoc.x, drawLoc.y, null);
                        g.setTransform(old);
                    } else {
                        paintObjectShape(g, mo, ox, oy);
                    }
                    final String s = mo.getName() != null ? mo.getName() : "(null)";
                    g.setColor(Color.black);
                    g.drawString(s, (int) (ox - 5) + 1, (int) (oy - 5) + 1);
                    g.setColor(Color.white);
                    g.drawString(s, (int) (ox - 5), (int) (oy - 5));
                }
            } finally {
                g.translate(
                        -bounds.x * tsize.width,
                        -bounds.y * tsize.height);
            }
        });
    }
}
