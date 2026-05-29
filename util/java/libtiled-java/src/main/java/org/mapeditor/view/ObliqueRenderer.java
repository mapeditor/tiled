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

import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.geom.AffineTransform;
import java.awt.geom.NoninvertibleTransformException;
import java.awt.geom.Point2D;

import org.mapeditor.core.Map;
import org.mapeditor.core.Tile;
import org.mapeditor.core.TileLayer;

/**
 * An oblique map renderer. Extends {@link OrthogonalRenderer} by applying
 * a skew transformation to the coordinate system.
 *
 * <p>Oblique maps use a skewed grid where the X and/or Y axis is projected
 * diagonally, controlled by the map's skewX and skewY properties.</p>
 *
 * @version 1.4.2
 */
public class ObliqueRenderer extends OrthogonalRenderer {

    private final Map map;

    public ObliqueRenderer(Map map) {
        super(map);
        this.map = map;
    }

    @Override
    public void paintTileLayer(Graphics2D g, TileLayer layer) {
        paintLayer(g, layer, () -> {
            final int tileWidth = map.getTileWidth();
            final int tileHeight = map.getTileHeight();
            if (tileWidth <= 0 || tileHeight <= 0) {
                return;
            }

            final AffineTransform transform = getTransform();
            final AffineTransform oldTransform = g.getTransform();

            final Rectangle clip = g.getClipBounds();
            Rectangle pixelClip = clip;
            try {
                AffineTransform inv = transform.createInverse();
                Point2D[] corners = {
                    new Point2D.Double(clip.getMinX(), clip.getMinY()),
                    new Point2D.Double(clip.getMaxX(), clip.getMinY()),
                    new Point2D.Double(clip.getMaxX(), clip.getMaxY()),
                    new Point2D.Double(clip.getMinX(), clip.getMaxY())
                };
                double minX = Double.MAX_VALUE, minY = Double.MAX_VALUE;
                double maxX = -Double.MAX_VALUE, maxY = -Double.MAX_VALUE;
                for (Point2D corner : corners) {
                    Point2D p = inv.transform(corner, null);
                    minX = Math.min(minX, p.getX());
                    minY = Math.min(minY, p.getY());
                    maxX = Math.max(maxX, p.getX());
                    maxY = Math.max(maxY, p.getY());
                }
                pixelClip = new Rectangle(
                    (int) Math.floor(minX), (int) Math.floor(minY),
                    (int) Math.ceil(maxX - minX), (int) Math.ceil(maxY - minY));
            } catch (NoninvertibleTransformException e) {
                // Fall back to the current clip when the transform is singular.
            }

            final Rectangle bounds = layer.getBounds();
            final int startX = Math.max(bounds.x,
                    Math.floorDiv(pixelClip.x, tileWidth));
            final int startY = Math.max(bounds.y,
                    Math.floorDiv(pixelClip.y, tileHeight));
            final int endX = Math.min(bounds.x + bounds.width,
                    (int) Math.ceil(pixelClip.getMaxX() / tileWidth));
            final int endY = Math.min(bounds.y + bounds.height,
                    (int) Math.ceil((pixelClip.getMaxY() + tileHeight) / tileHeight));

            g.transform(transform);

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

            g.setTransform(oldTransform);
        });
    }

    /**
     * Builds the skew (shear) transform based on the map's skewX and skewY values.
     */
    private AffineTransform getTransform() {
        final double tileWidth = map.getTileWidth();
        final double tileHeight = map.getTileHeight();
        if (tileWidth == 0 || tileHeight == 0) {
            return new AffineTransform();
        }

        final double skewX = map.getSkewx() != null ? map.getSkewx() : 0;
        final double skewY = map.getSkewy() != null ? map.getSkewy() : 0;

        final double shearX = skewX / tileHeight;
        final double shearY = skewY / tileWidth;

        AffineTransform transform = new AffineTransform();
        // AffineTransform.shear(shx, shy) applies:
        // [ 1   shx ] [ x ]
        // [ shy  1  ] [ y ]
        transform.shear(shearX, shearY);
        return transform;
    }
}
