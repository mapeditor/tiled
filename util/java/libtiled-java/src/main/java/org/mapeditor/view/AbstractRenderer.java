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

import java.awt.AlphaComposite;
import java.awt.Color;
import java.awt.Composite;
import java.awt.Graphics2D;
import java.awt.Image;
import java.awt.Point;
import java.awt.RenderingHints;
import java.awt.geom.AffineTransform;
import java.awt.Shape;
import java.awt.geom.Rectangle2D;

import org.mapeditor.core.MapLayer;
import org.mapeditor.core.MapObject;
import org.mapeditor.core.Tile;
import org.mapeditor.core.TileLayer;
import org.mapeditor.core.TileOffset;
import org.mapeditor.io.TMXMapReader;

/**
 * Base class for renderers that apply shared layer visibility/opacity rules.
 */
public abstract class AbstractRenderer implements MapRenderer {

    /**
     * Paints layer content while handling visibility and opacity consistently.
     *
     * @param g graphics context
     * @param layer the layer being painted
     * @param painter callback that performs the actual layer drawing
     */
    protected final void paintLayer(Graphics2D g, MapLayer layer, Runnable painter) {
        if (Boolean.FALSE.equals(layer.isVisible())) {
            return;
        }

        final float opacity = getOpacity(layer);
        if (opacity <= 0.0f) {
            return;
        }

        final Composite oldComposite = g.getComposite();
        if (opacity < 1.0f) {
            g.setComposite(AlphaComposite.getInstance(AlphaComposite.SRC_OVER, opacity));
        }
        try {
            painter.run();
        } finally {
            g.setComposite(oldComposite);
        }
    }

    protected final Point getTileDrawLocation(TileLayer layer, Tile tile, int baseX, int baseY) {
        return getTileDrawLocation(tile, baseX + getLayerOffsetX(layer), baseY + getLayerOffsetY(layer));
    }

    protected final Point getTileDrawLocation(Tile tile, int baseX, int baseY) {
        int drawX = baseX;
        int drawY = baseY;
        final TileOffset tileOffset = tile.getTileSet() != null ? tile.getTileSet().getTileoffset() : null;
        if (tileOffset != null) {
            drawX += tileOffset.getX();
            drawY += tileOffset.getY();
        }

        return new Point(drawX, drawY);
    }

    protected final void drawTileWithFlags(
            Graphics2D g,
            Image image,
            int drawX,
            int drawY,
            int flags,
            boolean hexagonalCells) {
        if (image == null) {
            return;
        }

        final int width = image.getWidth(null);
        final int height = image.getHeight(null);
        if (width <= 0 || height <= 0) {
            return;
        }

        boolean flippedHorizontally = (flags & (int) TMXMapReader.FLIPPED_HORIZONTALLY_FLAG) != 0;
        boolean flippedVertically = (flags & (int) TMXMapReader.FLIPPED_VERTICALLY_FLAG) != 0;
        final boolean flippedDiagonally = (flags & (int) TMXMapReader.FLIPPED_DIAGONALLY_FLAG) != 0;
        final boolean rotatedHexagonal120 = (flags & (int) TMXMapReader.ROTATED_HEXAGONAL_120_FLAG) != 0;

        double rotationDegrees = 0.0;
        double centerX = drawX + width / 2.0;
        double centerY = drawY + height / 2.0;

        if (hexagonalCells) {
            if (flippedDiagonally) {
                rotationDegrees += 60.0;
            }
            if (rotatedHexagonal120) {
                rotationDegrees += 120.0;
            }
        } else if (flippedDiagonally) {
            rotationDegrees = 90.0;

            final boolean originalFlipH = flippedHorizontally;
            flippedHorizontally = flippedVertically;
            flippedVertically = !originalFlipH;

            final double halfDiff = (height - width) / 2.0;
            centerX += halfDiff;
            centerY += halfDiff;
        }

        if (!flippedHorizontally && !flippedVertically && rotationDegrees == 0.0) {
            g.drawImage(image, drawX, drawY, null);
            return;
        }

        final AffineTransform oldTransform = g.getTransform();
        final AffineTransform transform = new AffineTransform(oldTransform);
        transform.translate(centerX, centerY);
        if (rotationDegrees != 0.0) {
            transform.rotate(Math.toRadians(rotationDegrees));
        }
        transform.scale(flippedHorizontally ? -1.0 : 1.0, flippedVertically ? -1.0 : 1.0);
        transform.translate(-width / 2.0, -height / 2.0);

        g.setTransform(transform);
        try {
            g.drawImage(image, 0, 0, null);
        } finally {
            g.setTransform(oldTransform);
        }
    }

    protected final void paintObjectBounds(Graphics2D g, MapObject object, double objectX, double objectY) {
        final Double objectWidth = object.getWidth();
        final Double objectHeight = object.getHeight();

        if (objectWidth == null || objectWidth == 0 || objectHeight == null || objectHeight == 0) {
            g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_ON);
            g.setColor(Color.black);
            g.fillOval((int) objectX + 1, (int) objectY + 1, 10, 10);
            g.setColor(Color.orange);
            g.fillOval((int) objectX, (int) objectY, 10, 10);
            g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
            return;
        }

        g.setColor(Color.black);
        g.drawRect((int) objectX + 1, (int) objectY + 1,
                objectWidth.intValue(), objectHeight.intValue());
        g.setColor(Color.orange);
        g.drawRect((int) objectX, (int) objectY,
                objectWidth.intValue(), objectHeight.intValue());
    }

    protected final void paintObjectShape(Graphics2D g, MapObject object, double objectX, double objectY) {
        final Shape shape = object.getShape();
        if (shape == null) {
            paintObjectBounds(g, object, objectX, objectY);
            return;
        }

        final Rectangle2D bounds = shape.getBounds2D();
        if (bounds.getWidth() == 0.0 && bounds.getHeight() == 0.0) {
            paintObjectBounds(g, object, objectX, objectY);
            return;
        }

        final AffineTransform oldTransform = g.getTransform();
        g.rotate(Math.toRadians(object.getRotation()), objectX, objectY);
        try {
            final Shape shadow = AffineTransform.getTranslateInstance(1.0, 1.0).createTransformedShape(shape);
            g.setColor(Color.black);
            g.draw(shadow);
            g.setColor(Color.orange);
            g.draw(shape);
        } finally {
            g.setTransform(oldTransform);
        }
    }

    private int getLayerOffsetX(TileLayer layer) {
        return layer.getOffsetX() != null ? layer.getOffsetX() : 0;
    }

    private int getLayerOffsetY(TileLayer layer) {
        return layer.getOffsetY() != null ? layer.getOffsetY() : 0;
    }

    private float getOpacity(MapLayer layer) {
        final Float opacity = layer.getOpacity();
        if (opacity == null) {
            return 1.0f;
        }
        return Math.max(0.0f, Math.min(1.0f, opacity));
    }
}
