/*-
 * #%L
 * This file is part of libtiled-java.
 * %%
 * Copyright (C) 2026
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

import java.awt.BasicStroke;
import java.awt.Graphics2D;
import java.awt.Point;
import java.awt.RenderingHints;
import java.awt.Shape;
import java.awt.geom.AffineTransform;
import java.awt.geom.Path2D;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import org.junit.Test;

import org.mapeditor.core.Map;
import org.mapeditor.core.MapObject;
import org.mapeditor.core.ObjectGroup;
import org.mapeditor.core.Orientation;
import org.mapeditor.core.Polygon;

public class HexagonalRendererObjectShapeTest {

    @Test
    public void testPolygonIsNotDrawnAsBoundingRectangle() {
        Map map = new Map(8, 8);
        map.setTileWidth(32);
        map.setTileHeight(32);
        map.setOrientation(Orientation.HEXAGONAL);

        ObjectGroup group = new ObjectGroup(map);
        map.addLayer(group);

        MapObject polygonObject = new MapObject();
        polygonObject.setName("");
        polygonObject.setX(60);
        polygonObject.setY(50);

        Path2D.Double shape = new Path2D.Double();
        shape.moveTo(60, 50);
        shape.lineTo(115, 63);
        shape.lineTo(90, 130);
        shape.lineTo(50, 95);
        shape.closePath();
        polygonObject.setShape(shape);

        Rectangle2D bounds = shape.getBounds2D();
        polygonObject.setWidth(bounds.getWidth());
        polygonObject.setHeight(bounds.getHeight());

        Polygon polygon = new Polygon();
        polygon.setPoints("10,0 65,13 40,80 0,45");
        polygonObject.setPolygon(polygon);
        group.addObject(polygonObject);

        BufferedImage image = new BufferedImage(
                map.getWidth() * map.getTileWidth(),
                map.getHeight() * map.getTileHeight(),
                BufferedImage.TYPE_INT_ARGB);
        Graphics2D g = image.createGraphics();
        g.setClip(0, 0, image.getWidth(), image.getHeight());
        g.setRenderingHint(RenderingHints.KEY_ANTIALIASING, RenderingHints.VALUE_ANTIALIAS_OFF);
        try {
            new HexagonalRenderer(map).paintObjectGroup(g, group);
        } finally {
            g.dispose();
        }

        Point rectOnlyPixel = findRectangleOnlyBorderPixel(polygonObject);
        assertNotNull(rectOnlyPixel);
        assertEquals(0, image.getRGB(rectOnlyPixel.x, rectOnlyPixel.y) >>> 24);
    }

    private static Point findRectangleOnlyBorderPixel(MapObject object) {
        final Shape shape = object.getShape();
        final Shape stroke = new BasicStroke(1f).createStrokedShape(shape);
        final Shape shadowStroke = AffineTransform.getTranslateInstance(1.0, 1.0).createTransformedShape(stroke);
        final Rectangle2D b = shape.getBounds2D();

        final int left = (int) Math.floor(b.getMinX());
        final int top = (int) Math.floor(b.getMinY());
        final int right = (int) Math.ceil(b.getMaxX()) - 1;
        final int bottom = (int) Math.ceil(b.getMaxY()) - 1;

        for (int x = left; x <= right; x++) {
            if (!containsAnyStroke(stroke, shadowStroke, x, top)) {
                return new Point(x, top);
            }
            if (!containsAnyStroke(stroke, shadowStroke, x, bottom)) {
                return new Point(x, bottom);
            }
        }

        for (int y = top; y <= bottom; y++) {
            if (!containsAnyStroke(stroke, shadowStroke, left, y)) {
                return new Point(left, y);
            }
            if (!containsAnyStroke(stroke, shadowStroke, right, y)) {
                return new Point(right, y);
            }
        }

        return null;
    }

    private static boolean containsAnyStroke(Shape stroke, Shape shadowStroke, int x, int y) {
        final double px = x + 0.5;
        final double py = y + 0.5;
        return stroke.contains(px, py) || shadowStroke.contains(px, py);
    }
}
